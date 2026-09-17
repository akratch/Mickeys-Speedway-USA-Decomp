# Mickey IDO 5.3 learnings

This is the reusable playbook for compiler behavior measured on Mickey. It is
not a function ledger and not a collection of folklore. An entry belongs here
only when a Mickey-exact match or a controlled experiment against Mickey's
pinned IDO 5.3 toolchain proves a principle that can guide another target.

Keep entries generic: state the symptom, mechanism, useful source lever, and
limits. Function-specific addresses, register triples, match percentages,
aligned instruction rows, and attempt history stay in `docs/resident.md`,
`docs/overlays.md`, `docs/matching-triage.md`, or ignored workbench state. ROM
bytes and disassembly never belong here.

## Diagnostic order

1. Prove the boundary, ABI, TU, configured flags, relocation surface, and
   untouched baseline before changing source.
2. Run the flag lattice before source permutation. Optimization level, ISA,
   debug mode, loop unrolling, the R4300 multiply scheduler, and the forced
   `uopt`-at-`-O1` path are measured project modes, not cosmetic switches.
3. Classify the first mismatch with the full-TU object and decomp-workbench.
   Use phase replay to decide whether C structure, `uopt` allocation, `ugen`
   allocation, or `as1` scheduling owns the residual.
4. Apply one source lever that matches the diagnosis. A higher score without a
   mechanism is not evidence and should not displace a better-understood base.
5. Rebuild through the configured project pipeline. Scratch/permuter scores
   are search hints; only exact owned bytes, relocations, linked placement, and
   the ROM comparison establish a match.

## Proven reusable behavior

### From a proved colour force to ordinary source

- **Symptom:** an accepted force removes a register-only residual, but the
  unforced web keeps choosing the lowest free colour. **Mechanism:** source
  reconstruction can erase an earlier interfering web by spelling a walking
  pointer explicitly, or merge an array base into a later data-pointer
  carrier. A value need not retain a separate emitted register use to affect
  global colouring: with strength reduction and call-result forwarding, a web
  can affect allocation without a separate use in the final instruction stream. **Lever:** inspect which values the
  source declares, then let indexed access generate the walking pointer while
  keeping the array base distinct from the loaded data pointer. Confirm the
  additional web and its interference in a fresh, fidelity-gated trace; do not
  infer them from a register census alone.
- **Symptom:** a cached floating-point scale beats an incoming parameter by
  exactly the weight of its out-of-loop definition. **Mechanism:** a named
  global load and a compiler-created literal have different identities and
  save totals. Treating a source literal as a mutable global plus a local
  carrier can introduce the entire priority deficit. **Lever:** authenticate
  the relocation's runtime section base and its addend, read the actual
  constant, and test the correctly typed literal directly at its use. Removing
  the carrier can remove that definition weight; when priorities tie, the
  parameter can precede the later-numbered constant web. A small stored addend
  alone is not a data address, and a plausible numeric value is not proof that
  a mutable global may be replaced by a literal.
- **Symptom:** an array cursor is already compiler-generated, yet an explicit
  pointer reaches its desired colour only with regressions elsewhere.
  **Mechanism:** a different named induction variable can determine the
  cursor's allocation order. Removing an independent array index can also
  remove interference with an apparently unrelated constant, even when that
  constant's saving priority is unchanged. **Lever:** express a bounded
  coordinate recurrence directly as base plus index times stride, then let
  indexed array access generate the cursor too. Retain any independent index
  needed by the existing control flow. In the
  [menu source controls](whale-source-colours.md#lane-wv-n-2026-09-14), generating
  the coordinate repaired colours; removing the compensating explicit pointer
  then restored load ordering. Check both steps separately: an explicit pointer
  and a named-array subscript can schedule differently. This applies only when
  the arithmetic is defined and the base and stride remain stable across the
  loop; it does not justify caching mutable global loads across calls.
- **Validation and limits:** isolate the changed decisions with retained source
  and trace evidence, then compare aligned residual windows, frames and
  instruction geometry.
  Generating an induction pointer can add a temporary home; removing the
  cached scale can recover it and change the schedule again. Declaration-order
  experiments on the old shape do not close the new shape. Any emitted
  literal pool must be proved against retained data, with only declared
  metadata rebinding and ordinary linking; the compiler's instruction bytes
  must remain untouched. These levers are not universal instructions to delete
  locals or replace globals. Evidence: the ROM-exact recipe in
  [the angle-scan handoff](matching-triage-handoffs/overlay1FindType47ByAngle.md).

### Flags and compiler phases

- The driver does not run `uopt` below `-O2`, but Mickey's
  `setglobalintmask` requires `uopt` forced into an otherwise `-O1` pipeline.
  `tools/ido-phases.py` is therefore a real flag-lattice member, not a debug
  convenience. Evidence: `docs/modules.md` section 6's flag census and
  `docs/flag-sweep.md`.
- A downstream scheduling diff can originate in an upstream register choice.
  Diagnose and replay the producing phase before attempting statement-order
  churn; forcing a target color is causal diagnosis only and is never a
  promotable build step. Evidence: `docs/breakthrough-campaign.md` Phase 2.
- Per-TU flags stay local. One exact function can establish a candidate flag
  group, but changing a shared group requires evidence and an impact review
  across every consumer. Evidence: ADR 0007 and `docs/flag-sweep.md`.
- `-Wab,-r4300_mul` can alter FP multiply hazard scheduling as well as the
  integer-multiply cases its name suggests. When otherwise exact MIPS-II code
  is one scheduler nop short between adjacent FP multiplies, test this flag as
  a focused mechanism probe; keep it only after every function in the shared
  TU, all relocations, and the linked image remain exact. Evidence: Overlay
  25's exact effect initializer in `docs/overlays.md`.
- The R4300 multiply scheduler can resolve an adjacent transposition at zero
  size delta without inserting a hazard nop. A fixed scheduler-priority
  argument does not close a residual until the relevant assembler flag is
  tested. In a controlled replay, identical ugen output and symbol metadata
  produced different schedules solely from as1's `-r4300_mul` option; the
  flagged replay equalled the stock flagged C object in text and relocations.
  Test `-Wab,-r4300_mul` before further source-order searches around multiply
  instructions, even when the size and opcode inventory already agree.
  Keep the flag local and prove all TU consumers and linked bytes before
  adoption; a diagnostic replay never supplies matching build output.
- A per-TU flag adopted mid-iteration can be byte-inert by the time the source
  lever lands, and loop unrolling is the common case. IDO's rotator peels a
  loop head into the preheader when a separate start-value copy keeps the
  counter live across it, and the unroller then emits a wide body plus a
  runtime remainder prologue. Folding the start value into the counter itself
  removes the peel, after which `-Wo,-loopunroll,0` has nothing left to
  suppress. Re-test any flag adopted while the source was still wrong:
  rebuild the TU with and without it and compare every function's `.text`
  bytes symbol by symbol, in both the canonical and `-DNON_MATCHING` builds.
  Object hashes are not the test; the driver embeds the asm-processor's
  temporary preprocessed-file name, so two objects built from identical
  inputs differ outside `.text`. Pair the test with a positive control that
  drops a flag known to move that TU's codegen, so an all-identical result is
  a measurement rather than a broken comparison. An inert flag may stay as
  the recorded constraint, but it is not a fresh lever for the TU's other
  unmatched functions and their residuals must not be attributed to it.
  Evidence: overlay 8's translation unit, where all 21 functions compile to
  identical `.text` with and without `-Wo,-loopunroll,0` and the ROM verifies
  either way, while dropping `-Wab,-r4300_mul` as the control moves seven of
  them.
- **At `-O2` IDO moves at most two GPR-passed `f32` formals with `mtc1`; the
  third always goes through its argument home.** Under O32 a `float` parameter
  that follows an integer or pointer parameter arrives in `a1`/`a2`/`a3` as raw
  bits and has to cross into an FP register. With one or two such formals every
  one becomes an `mtc1`. With three, `-O2` converts two and lowers the third as
  `sw` to its home plus `lwc1`, which is one extra word, and no source form
  changes it: 2-D versus flat indexing, an explicitly typed pointer parameter,
  the `register` storage class, copying the parameters into locals, and cutting
  the body to a single product per formal all give the same 2-of-3 split, at
  both `-mips1` and `-mips2`. `-O3` converts all three and the function is one
  word shorter. So a target whose prologue shows three `mtc1` of `a1`/`a2`/`a3`
  is either an `-O3` object or not compiled from C at all, and that is
  decidable from the fallback before any source work. Measured on an isolated
  three-line probe and on `func_8002B040` in `src/main/matrix.c`, where `-O3`
  takes the instruction census delta to zero. Limit: this is about formals
  arriving in integer registers, not about `f12`/`f14` FP arguments, and it
  does not license adopting `-O3` for a shared TU -- ADR 0007 still wants an
  exact function first.

- That same flag is decidable from the target bytes *before* any source work,
  and the test is cheap: disassemble each unmatched function's own fallback
  and count the scheduler nops that sit between two adjacent single-precision
  multiplies. A translation unit compiled without the flag cannot emit one at
  all, so a nonzero count anywhere in the unit settles the question for the
  whole object. Read the whole unit, not one function: functions with no
  adjacent FP multiply pair are silent either way, and a unit's already-matched
  functions can be silent too -- a passing `gmake verify` after adding the flag
  is therefore consistency evidence, never proof. Expect candidates that were
  shaped against the wrong scheduler to score slightly worse once it is
  corrected; that is the candidates being wrong, not the flag. Evidence: the
  shadow TU in `docs/resident.md`, where three unmatched functions carry eight,
  four and two such nops.

### Retained data and relocations

- An exact internal branch can lack the assembler fallback's PC16 record
  because IDO already resolved its displacement. Restore metadata only when
  the existing displacement and destination are independently authenticated:
  a same-section symbol at the branch site makes the link contribution zero.
  Assert the opcode, unchanged addend, symbol position, in-range destination
  and executable-prefix digest. This does not repair a wrong branch field or
  establish identity from instruction equality alone; require the original
  relocation tuples and linked owned-ROM comparison. Evidence: the resident
  object switch promotions in `docs/reloc-surface.md`.
- When a mixed translation unit emits the exact instruction fields but also
  creates a duplicate compiler-private literal pool, preserve the fields and
  rebind only their existing relocations to one absolute symbol for the
  retained overlay-local pool. Externalize the duplicate section only behind
  an exact payload digest, and require the complete runtime relocation surface
  plus linked ROM bytes to agree. This applies only when the source object's
  words are already exact after ordinary relocation; moving an addend into a
  LO16 field after compilation remains prohibited. Compiler-private jump-table
  relocations may use the same contract when the duplicate table's relocation
  section is removed as metadata and the retained payload is independently
  authenticated. Evidence: Overlay 8's exact channel updater and Overlay 41's
  exact item spawner in `docs/overlays.md`.

- A multiword counter update can have correct arithmetic but the wrong address
  materialization when a donor's separate read and write names are collapsed
  into one extern. Under the measured resident IDO settings, a TU-defined
  destination and a separate extern read reproduce a different load/store
  sequence. An extern destination, including a fixed-size array, still emits
  an additional high-half address materialization for the second store.
  A native weak read alias of the actual TU-defined object can preserve the
  donor's spelling while binding both accesses to the same storage. Use this
  only when the donor and target establish that identity: prove the linked
  alias address, every relocation, initializer and owned data extent. Migrating
  the real initializer may require trimming only compiler alignment padding;
  a second allocated object or a masked instruction score is not a substitute
  for proving shared storage and the full ROM. Evidence: the scheduler
  retrace reconstruction in [the resident census](resident.md).

### Allocation and source shape

- **Price a forced loop constant together with the other rematerialized webs.**
  When a target restores a constant at many handler exits but the candidate
  materializes it only a few times, an accepted colour force can recover those
  restorations while making a shared address remain allocated. Its address
  materializations then disappear, and the whole function can become shorter
  even though the constant's own inventory grows. Count both inventories and
  their relocation surfaces in the forced object before translating a decision
  into a missing-word price. Re-price on the current source shape; a successful
  constant force in a neighbouring function does not establish independence.
  The useful lever is the coupled live-range/split decision, not inserting
  literal restorations into the C. Limits: the force is diagnostic, the desired
  colour may still be forbidden, and no object from a forced compiler is
  promotable. Evidence: the controlled baseline and revised-shape forces in
  [the animation command handoff](matching-triage-handoffs/func_800517E0.md)
  and the contrasting positive net constant price in
  [the collision-update handoff](matching-triage-handoffs/func_80053868.md).

- A declared local reserves a frame home whether or not it is register
  coloured, and the declared block is laid out at the TOP of the local region
  in declaration order, first-declared highest, with the compiler's own
  temporaries below it. Two consequences are levers. First, a decompiler draft
  that names every intermediate pays frame for each name, so an oversized frame
  is a declaration census before it is an allocation problem: rewriting an
  intermediate as an expression moves that value into the compiler-temp region,
  which is where the target usually keeps it. Second, once the declaration
  count is right, declaration ORDER decides which displacement each surviving
  home takes, and a value the target homes at a particular offset pins its
  position in the list; a sweep over orders moved one function's frame across
  three different sizes with a byte-identical body. Limits: this is a frame and
  home lever, not a colour lever. It does not apply where the frame difference
  is in the callee-saved half, and where a local is promoted into a callee-saved
  register its save slot is its home, so the census does not shrink the frame.
  Evidence: the character-control candidates in `docs/matching-triage-handoffs/`, one
  of which reached an exact frame, exact instruction count and an identical
  stack map from a 0x38-byte excess.

- The census lever needs no `-g3`, and it closes functions on its own. Two
  resident translation units built at the project default (no debug flag) were
  taken from an operand-only residual to exact by reordering declarations and
  changing nothing else: one where a spilled retry accumulator wanted the
  fourth position rather than the last, and one where a helper's floating
  result wanted the sixth of seven. In both the frame, the opcodes, the
  registers and the relocations were already exact and every differing word was
  the same displacement, which is the signature to look for: an operand-only
  residual whose sites are all stack displacements is a declaration census
  question, and reading the wanted homes back through `frame_top - 4k` gives
  the position directly. Positions the target does not spill are free, so the
  lattice usually has many solutions and any one of them can carry natural
  names. Evidence: `rumbleTick` in `src/main/saves.c` and `func_8004FAD0` in
  `src/main/block_506D0.c`.

- A declaration the compiler never materialises can still be load-bearing, and
  the two facts about unused declarations are unit-specific. In the perspective
  helper above, one of the seven declared locals is never read or written in
  the emitted code, yet deleting it shortens the frame by eight bytes and moves
  twenty words; the dead store a decompiler draft aimed at it is not needed,
  but the declaration is. This is the opposite of overlay 8's reading, where an
  unused declaration reserved no home at all, so do not carry either result
  between units: measure the frame with and without the declaration before
  deciding whether it belongs.

- Limit on the census lever: declaration order can be a colour lever without
  being a frame lever, so measure the frame before spending a sweep on it.
  Overlay 8's `overlay_008.c` was swept over twenty-one whole-list orders across
  two functions -- every 4-byte scalar moved individually between the two
  aggregates, the address-taken pair moved together and apart, the sub-word local
  moved through four positions, and both aggregates exchanged -- and the frame
  never moved once, at `0x90` and `0x70` respectively, while the score ranged from
  56 to 80 words. In the same unit, removing a declared local does not shed frame
  bytes, an unused declaration reserves no home at all, and the `register` storage
  class is byte-inert in four placements. Where the excess frame is not in the
  declared block, order search only redistributes colour and will read as a long
  run of same-frame near-misses; the tell is that a whole family of permutations
  collapses to one identical object. Evidence: the four overlay-8 handoffs in
  `docs/matching-triage-handoffs/`.

- Repeating a memory read that a named local already holds is not a wasted
  instruction: it is how the target keeps the loaded value in a caller-saved
  carrier while the named local takes the callee-saved one. Where a draft
  assigns a field to a variable and later reuses that variable, and the target
  instead shows the field loaded into a scratch register and copied into the
  saved register, spell the later use as the field access again. The values are
  identical, so the optimiser folds the second access into the first as a common
  subexpression, but the two source references keep two live carriers and
  produce the copy. Limits: the two references must be provably the same value
  with no intervening store to that address; and prefer this over adding a
  second declared local, which pays a frame home and can push a stack-passed
  parameter into a callee-saved register it does not occupy in the target.
  Evidence: the character-control jet-flame candidate in
  `docs/matching-triage-handoffs/`, where this recovered the single missing
  instruction that had been shifting every later relocation by one index.

- A stack-passed fifth argument that the target re-reads from its incoming home
  at every use is a register-pressure readout, not a spelling to force. Adding
  declarations to such a function can flip the argument into a callee-saved
  register and displace a value the target keeps there, growing the frame and
  the residual together. Read the argument's home traffic in the target first,
  then keep the candidate's declaration count at or below the level that leaves
  the argument in memory. Limits: this diagnoses the pressure, it does not set
  it; volatility and other qualifiers on the parameter are not a policy-clean
  substitute. Evidence: the character-control jet-flame candidate in
  `docs/matching-triage-handoffs/`.

- Donor local lifetimes and empty statement boundaries can be coupled. In a
  configured full-TU comparison, restoring a global re-read across a conditional
  call corrected the relocation count but worsened geometry and register
  allocation. Restoring the same donor's empty statement boundary then made
  bytes and relocation identities exact. When an authorized donor supplies
  both forms, test the pair before treating the first edit's regression as
  evidence against the complete donor spelling. This proves source-form
  sensitivity, not which compiler phase owns it; an empty constant statement
  does not impose a runtime memory barrier. Preserve the game's call/read order,
  disclose any retained inert form, and require full linked proof. Evidence:
  the resident video mode changer in [resident §3.8](resident.md#38-gamevi-rom-0x341800x34e60).
- The declared-local list is a frame instrument with two independent effects,
  and both are measurable in one build. Its length sizes the local block in
  8-byte steps, so a frame that is N bytes too large is N/4 declarations too
  many and a deleted decompiler-only temporary is worth exactly one step. Its
  *order* then fixes where each spilled local lives: homes descend from the top
  of the local block in declaration order, so the k-th declaration owns
  `frame_top - 4k`. Census the target's stack displacements first, decide which
  source variable each one holds, and order the declaration list so those
  variables land on the measured offsets; every remaining declaration is free
  to sit anywhere. Limits: only spilled locals reveal a home, so the census
  constrains a subset of the list, and locals past the block's capacity get no
  home at all while still counting toward the length. A local that the body
  never reads still reserves its slot, which makes the length adjustable
  independently of the code -- useful as a probe, not as a finished body.
  Evidence: the shadow-projection query in `docs/resident.md`, where five
  homes and the frame were closed by reordering alone.
- Referencing a global twice makes IDO materialize its address once into a
  colored register and load through it; referencing it once folds the address
  into the load itself. When the target forms an address and then loads from
  offset zero through it, and especially when it reloads through that same
  register after a call, the source spelled the global at both sites rather
  than caching the loaded value in a local. Caching it in a local instead keeps
  the *value* live across the call and both shortens the address's live range
  and adds a local; spelling the global twice reproduces the reload the target
  performs. Limits: this is about the address web, not aliasing -- IDO reloads
  the value after a call in both spellings.
- A value that only one deep path consumes is sunk to that path even when the
  source computes it early, which keeps its inputs alive across everything in
  between. Hoisting the statement in the source does not stop it, and neither
  does splitting it across two locals. Two spellings do: assigning the result
  back to the variable that produced it, or reading the underlying memory field
  directly at both the test and the computation instead of through a local. The
  second is the one to reach for when the target's own shape shows the
  computation up at the test, filling a branch delay slot that the sunk form
  leaves as a taken-branch reload. Limits: the memory-read spelling only works
  when the field is genuinely re-readable at both points.
- Two spellings of the same element address compile differently and the target
  says which one the source used. Pointer arithmetic over a struct whose size
  is not a power of two strength-reduces to a shift/add chain against the
  element count; an indexed access to the same struct multiplies by a stride
  the loop hoists into a register. A candidate that computes every offset by an
  explicit byte multiply gets the register form everywhere and cannot produce
  the shift chain, so declare the element type and let the two spellings
  separate. The same distinction decides whether an address is common-subex-
  pressed with a nearby load's address: a load's address is lowered in a later
  phase than an ordinary expression, so writing the argument as an indexed
  element rather than reusing the offset variable keeps them apart.
- A call argument computed from a value that was *just stored to memory* costs
  an extra register-to-register copy, and which memory it is read from is the
  whole lever. Where a function stores a parameter into a global and then
  passes a function of that value, computing the argument from the **parameter**
  lets the compiler write the result straight into the argument register,
  because nothing needs the parameter's carrier afterwards. Computing it from
  the **global** the parameter was just stored to keeps the stored value's
  carrier live across the computation, so the result takes a temporary and is
  copied into the argument register at the call. The two spellings are the same
  value and differ by exactly one instruction. Read the target first: a
  computation that lands directly in the argument register says the source used
  the parameter; a temporary plus a copy says it re-read the global. This is
  distinct from the aliasing question -- there is no store between the two
  spellings that could invalidate anything -- and from the "reference a global
  twice" address rule, which is about the address web rather than the value's.
  Evidence: `func_8000D3B8` in `src/main/track.c`, where the pool size spelled
  from the global took the function from one instruction short and 105
  differing words to exact, and where four rewrites of the *arithmetic* and all
  four statement positions were flat.

- Storing a narrow struct field and then reading that field back is how one
  carrier serves both a store and a later use of the same value. Where a
  function assigns a `u8` or `u16` field from a wider local and later needs the
  same value, spelling the later use as the **field** rather than the local lets
  uopt forward the store, so the field's own truncation mask is the only extra
  operation and no second load appears. Spelling both uses from the local
  instead makes each intervening store through the struct pointer kill the
  available load, and the value is re-read once per use. The tell in the target
  is a mask by the field's width sitting at a use where the source has no mask
  of its own: that mask is the forwarded word being truncated to the field type,
  not something the programmer wrote. Limits: the field and the local must be
  provably the same value at that point, and this trades a load for a mask, so
  it only shortens the function when the field is read more than once.
  Evidence: `func_80019DE8` in `src/main/lights.c`, where three reloads of a
  stack-homed parameter became two, closing a +4 size mismatch and taking the
  residual from 45 words with nine structural differences to 16 register names
  -- a pure temporary-ring rotation, closed by the ring entry below.

- A decompiler's copy variable is often the compiler's own live-range split,
  and declaring it costs an instruction. Where a draft carries `var_sN = var_sM`
  and both names are equal on every path afterwards -- each branch assigning one
  from the other -- the second name is not a source variable: it is the copy the
  allocator makes to keep the value in a caller-saved carrier across the tests
  while the callee-saved one stays live to the next use. Declaring it makes the
  compiler materialise a *second* carrier and copy into it as well. Folding the
  two names into one is semantics-preserving under that equality and lets the
  compiler place its own copy where it wants it. Limits: the fold frees a
  callee-saved register, which can change loop-invariant hoisting -- expect the
  saved-register assignment to rotate and one more constant to be hoisted, and
  measure the frame and the hoist set, not only the instruction count.
  Evidence: `func_80046BCC` in `src/main/diCpu.c`, +4 to exact size and 89
  differing words to 41.

- Statement order, not declaration order, drives which web is colored first
  inside a block. Moving an assignment ahead of its neighbours changes the
  register both of them receive; permuting the declaration list does not. Use
  the declaration list for homes and statement order for colors, and do not
  spend attempts permuting declarations to chase a register.
- A byte used both as a table index and as a stored selector can create an extra zero-extension instruction if some reads are signed and others unsigned. Make the field unsigned when the target loads it that way, or keep the unsigned conversion consistent at every use. A conversion only at the lookup can leave a shared signed load and a separate mask. Preserve the table index range and byte-store behavior; require the full configured object and relocation/ROM proofs. Evidence: the exact resident distance-tier dispatcher in `docs/resident.md`.
- Reusing an expired loop-counter local for a later dispatch selector can change the selector and constant carriers even with no frame change. Keep the saved model index as a separate value read from its object field, and reuse only the counter whose earlier value is dead on every path. This repaired the later dispatch allocation in an exact resident object routine. It is a measured source-shape lever, not proof of a particular allocator phase. Evidence: the exact resident distance-tier dispatcher in `docs/resident.md`.
- A command-pointer load followed by a separate cursor update can produce a
  different allocation from `command = (*cursor)++`, even when both advance
  by exactly one command. Paired full-TU traces showed the post-increment form
  create an additional interfering web in p1; the stock command instructions
  then used the required color. The resulting two-store scheduling residual
  closed by placing the independent header and payload assignments on one
  physical source line. Preserve the pointer type, increment size, evaluation
  count, and complete command writes. A forced color alone changed the store
  order and was not exact, so remeasure the whole sequence after changing
  allocation. This is a measured source-form lever, not a rule that every late
  return-register mismatch comes from a callee prototype. Evidence: the exact
  resident sprite-segment renderer in `docs/resident.md`.
- Correct frame size does not prove the source's stack objects. A callee that
  reads and writes a full vector, or consumes a transform with a later frame
  field, can expose a draft's undersized arrays and disconnected locals.
  Reconstruct those aggregates from the callee's authenticated accesses before
  tuning homes or colors; separate volatile scalars are not a substitute for
  the object actually passed. Reusing a consumed input parameter for a later
  result can then remove a surplus home when the target stores that result in
  the parameter's caller home. Prove that no subsequent path needs the input
  value, and validate every read field and aggregate extent. Evidence: the
  exact resident sprite-segment renderer in `docs/resident.md`.
- When a table index local adds an unwanted colored value before a call,
  carry the final table result into the call instead. A nested lookup assigned
  before argument setup can leave the intermediate index temporary while the
  result takes its call-argument register at zero measured color cost. Paired
  full-TU traces with stock-output fidelity confirmed this in the exact resident
  ROM-section DMA helper recorded in `docs/resident.md`. Preserve signed index
  widening, lookup order, and evaluation count. Directly nesting the lookup in
  the call or overwriting the incoming parameter can instead add a spill or
  store; neither is equivalent as an allocation experiment. This is a carrier
  placement lever, not proof of a target register's allocator role from its name.
- When a masked unsigned bit controls a branch, spelling the test as an
  unsigned left shift followed by a right shift can change UGEN's temporary
  phase even when optimization folds the final right shift out of the branch
  schedule. In one exact switch dispatcher, this natural bit extraction kept
  every opcode and relocation site fixed while repairing a shared temporary
  carrier. Apply it only when the source object's unsigned width and selected
  bit are proved; a signed shift, wider type, or value-consuming expression is
  not equivalent. Require exact configured words, frame, relocations, linked
  owner, and full ROM. Evidence: Overlay 7's exact mode dispatcher in
  `docs/overlays.md`.
- A small register-only switch residual can come from a named selector even
  when both functions are frameless. Putting a single-use selector expression
  directly in the switch removed its named carrier and restored exact output
  under unchanged full-TU flags. Try this only after measuring the frame and
  relocation surface, and preserve the expression's integer conversions and
  evaluation count. The paired source result proves allocation sensitivity,
  not which compiler phase caused it or a general rule for all switches.
  Evidence: the exact object-data sizing switch in `docs/resident.md` and its
  configured relocation and ROM proof in `docs/reloc-surface.md`.
- Possible colorability is not per-function ownership. The workbench's static
  IDO 5.3 temporary-only classification of integer t0 through t5 conflicts with
  its own decoded color map and with a controlled Mickey function whose count
  and next-frame values occupy colored t0 and t2. Do not use that profile as
  proof that a target register is uncolorable or that UGEN alone owns a residual.
  Bind actual role evidence separately to each compared input; missing target
  evidence must remain missing. A FIFO replay over a subset chosen from observed
  allocation returns proves only consistency with that subset, not that it is
  the complete available pool. Check initialization and reservation semantics
  independently, and never infer reservation merely from absent allocations or
  absent surviving instructions. A later source carrier can change early output
  while early expression records remain unchanged, so demand order is not the
  only explanation to test. Evidence and the withdrawn overclaim are recorded
  in the `func_80020D8C` two-source handoff. Workbench commits `028a8eaf` and
  `1da7f333` now separate possible colors, conditional per-input reservations,
  and trace-directed ownership; independent review and real-input acceptance
  preserve the scalar comparisons while withholding unsupported causal advice.
- A traced free request is not necessarily a queue transition. In the pinned
  UGEN producer, FREE/FORCE_FREE entry hooks precede conditional mutation, and
  a successful free can append directly without invoking the separate ADD hook.
  REMOVE is also conditional on membership; MOVE_END belongs to the used list,
  not the free FIFO. Source-authenticated initial ADD/REMOVE records established
  different initial membership in a controlled pair, but did not make its
  entire dynamic queue history observable. Dropping requests and relying only
  on ADD would lose real frees; treating every request as an append invents
  others. Keep initialization proof separate from complete replay, and require
  successful-transition evidence for the latter. The compiler-source binding
  and limits are recorded in the `func_80020D8C` reservation audit.
- Reusing a masked index in an existing predicate can change its allocation
  class, not merely its temporary demand order. In a controlled full-TU pair,
  an equivalent predicate shared the low mask with a table access. Faithful
  stock/trace-off/trace-on output and named procedure traces showed that the
  shared value became colored across the branch, its separate body calculation
  disappeared, and GP allocator-result events decreased. Switching only between byte
  arithmetic and typed indexing still swapped the pointer/scale temporary roles
  in both variants. Diagnose these two effects separately: tree order can be
  controllable while cross-branch reuse destroys the target's instruction shape.
  A predicate equivalence needs its complete input/path domain proved; do not
  infer storage validity from a bounded index or add observable guards. This
  negative control is not an exact-source recipe or target compiler trace.
  Evidence: the predicate-demand controls in the `levelFreeAll` handoff shard.
- A swap of two temporary-register roles need not be a FIFO phase error.
  Final scheduling can hide which expression requested each temporary first.
  Faithful stock/trace-off/trace-on controls and unchanged stock pass replay
  showed identical FIFO initialization and return sequences, but different
  expression-to-return assignments. Moving an unused cursor offset from a
  call-result initializer into an existing nonempty-range arm put the bounds
  demand before the index-scale demand in both pre- and post-optimization
  streams and closed the register-only residual. Inspect demand order before
  adding a phantom allocation or pursuing colored-register changes. Preserve
  the unconditional helper call, output writes and all observable accesses;
  postpone arithmetic only when it is unused on the excluded path and defined
  on the retained path. This is not proof of the target's original source or
  compiler trace, nor a claim that the intermediate streams differ only by
  ordering. Exact output still depended on retained predicate grouping; an
  unused declaration and equivalent relational spelling were independent flat
  controls. Evidence: the matched record lookup in
  `src/overlays/o001/overlay_001.c`, with its exact mixed-TU and linked-ROM range
  recorded in the overlay atlas and remaining spelling in the cleanup queue.
- Instruction order inside a basic block is decided by `as1`'s list
  scheduler, not by `ugen`, and among ready instructions it prefers the
  lower source line stamped on each emitted record. A loop-invariant that
  `ugen` hoists (a table or base address, a loop constant) carries the loop
  header's line, so an initializer written on the line above the loop is
  scheduled before the hoisted address even when the target executes it
  after. Placing the initializer on the header line (`count = 7; do {`)
  moves it behind the hoist; conversely, splitting a loop-header initializer
  onto the preceding line can move the initialization ahead of the branch and
  leave the hoisted address as its delay-slot fill. Evidence:
  `overlay40UpdateEntries` (44/46 to exact), the three scheduled words of
  `overlay57HandleModeInput`, and the exact resident texture-table initializer.
  The first two were found with the workbench's ugen emit-provenance trace on
  2026-09-02. It does not apply to register renames, to delay-slot fills chosen
  by latency (`func_8001A154`'s `li -1`), or to relocation-surface differences.
- The lexical start of a conditional block can be an allocation boundary for
  a loop-invariant expression. If IDO hoists an invariant value into a saved
  register, adding save/restore and move instructions, initialize the existing
  local at the start of the only branch that consumes it; IDO may keep a
  per-iteration temporary and recover the smaller frame. This is valid only
  when the rejected path never observes the value and the ordinary object,
  relocations, linked owner, and ROM all remain exact. Evidence: Overlay 34's
  exact record updater in `docs/overlays.md`.
- Equivalent nonzero tests can affect register selection for an unsigned
  halfword local. A controlled paired full-TU IDO 5.3 build with `-O2 -mips2`
  changed only `if (value)` to `if (value != 0)`, preserving the input pathname,
  every line break, declarations, and flags. Only register fields changed;
  instruction geometry, frame, relocations, neighboring functions, and data
  stayed identical. When a narrow condition is implicated in a small allocation
  residual, isolate these natural spellings before changing unrelated locals.
  This proves source-shape sensitivity, not which compiler phase causes it;
  the effect is not established for other scalar widths, flags, or TU contexts.
  Evidence: controlled paired compiler experiment, 2026-09-06. Matching still
  requires the ordinary configured object and complete linked-ROM proof.
- The NUMBER of declared locals, not only their order, sets where a
  call-crossing value is homed: declared locals take descending homes from
  the frame top in declaration order and compiler temporaries sit below
  them, so removing a named local and letting a call-crossing common
  subexpression stay a compiler temporary moves it one word lower
  (`overlay34InitStorage`), reusing an existing local as the carrier drops
  the declared count by one and lowers a later spill by one slot
  (`func_overlay_026_F0000B18_187AF10`), and declaring a pair after the
  local whose slots it must follow lands both on the retail homes
  (`overlay84InitializeAndUpdate`). All three were exact on 2026-09-03.
- A never-read scalar declaration may still displace compiler-created stack
  homes even when its initializer is optimized away. When opcode, register,
  frame, and relocation surfaces already agree and every residual is the same
  one-word home displacement, remove only a local whose initializer is proved
  defined and side-effect-free and whose value is never observed. In an exact
  resident FX closure this moved three call-crossing homes by one word without
  changing any other instruction. Recheck the ordinary object, complete
  relocation identities, linked owned range, and full ROM; this lever does not
  apply to locals whose initialization or lifetime is observable. Evidence:
  the exact `func_8004ADE8` closure, 2026-09-08.
- Removing one redundant input alias can disturb exact stack homes while
  removing two together recovers both the frame and the surviving homes.
  Declaration placement can expose the intermediate state: a correct frame
  and named locals, with only compiler temporaries displaced by one word.
  Removing the remaining alias can then move those temporaries without
  changing the rounded frame size. A regressing single-alias deletion does
  not rule out that paired change when new frame and home measurements
  support it. Apply only to aliases that are never modified or escaped;
  preserve the parameter types, call order and memory accesses, and require
  ordinary compiler bytes, relocation identities and linked-ROM proof.
  Evidence: the Overlay 26 effect-handler closure in `docs/overlays.md`,
  2026-09-08.
- Used scalar copies can recover an omitted declared stack region without
  retaining the memory traffic of an array. For a movement operation that
  consumes each displacement twice, separate float copies preserved the
  instruction and register sequence while enlarging the frame; an equivalent
  three-element array retained extra loads/stores. Once the frame agreed,
  moving a used bound declaration before the call-crossing coordinates placed
  their homes. Distinguish frame extent from home position and measure them
  separately. This is a source-level layout observation under stock IDO 5.3
  `-O2 -mips2`, supported by configured object and linked-ROM identity; it does
  not justify unused padding or new volatile accesses.
- Declaration position can shrink the emitted frame without reducing the
  declaration count. Moving a spilled return pointer earlier recovered both
  the frame and its home while later declarations remained in the source.
  Distinguish the front end's reserved offsets from the final frame: declaration
  count alone is not a lower bound after optimization. Measure actual stack
  accesses and frame extent before deleting useful locals. This was proved
  with a paired stock full-TU IDO 5.3 build and subsequent linked-ROM identity;
  the available web-detail trace covered only part of the frame and did not
  establish which pass removed the unused tail region.
- A register swap confined to a later loop can come from a local shared with
  an earlier loop. In a clear-then-copy routine, using the copy source as the
  clearing cursor joined both lifetimes into the higher-priority pointer web.
  A forced color merely moved the residual into the clear loop. Reusing the
  destination for both writing phases instead left the read source local to
  copying and made stock output exact. Paired fidelity-proved p1 traces kept
  the same two priority levels but exchanged which source home occupied each.
  Inspect every use of the shared local before treating a late swap as an
  isolated tie-break; this lever requires real, compatible pointer roles and
  does not justify new accesses or forced compiler output in canonical builds.
- A one-local ablation can move temporary spill homes while leaving the frame
  unchanged because the frame rounds to eight bytes. In a loader whose register
  and instruction sequence already agreed, removing redundant aliases in pairs
  reduced the frame while restoring every emitted spill offset. Direct repeated
  array expressions preserved common-subexpression carriers without the named
  locals; deriving one byte offset from another changed those carriers and
  regressed allocation. Read frame extent and spill offsets separately, and
  preserve independent index expressions when removing their aliases. This was
  measured with stock full-TU IDO 5.3 builds and confirmed by linked-ROM identity;
  it does not establish the same behavior for addressed locals or expressions
  with intervening writes or calls.
- A mixed integer/pointer sentinel can cost both a declared home and a second
  constant carrier even when the stored bits are identical. On a proved
  32-bit raw-word table, model each physical word with an integer/pointer
  union, reuse a pointer parameter only after its original value is dead, and
  write both sentinel words through the same pointer member. IDO then keeps one
  typed constant web; combining that with repeated scaled indexing can remove
  the named index and sentinel homes together, reducing the frame by one
  eight-byte quantum while retaining the call-crossing spill. This is valid
  only when the word and pointer widths and all-ones representation are proved,
  the original pointer has no later use, and the ordinary object, complete
  relocation surface, linked owned range, and full ROM are exact. Evidence:
  the exact `func_800347A0` closure, 2026-09-08.
- ugen's integer temp ring is consumed one pop per compiler temporary, and
  the `DKWB_UGEN_TRACE` pop sequence shows the count directly: reading a
  struct field through a local costs a pop that a direct read does not, an
  index scaled twice costs one more pop than an index scaled once (type the
  table as pairs), and a pool-carried accumulate (`x = a; x += b * c;`)
  changes a pop. Two pops off means two of these, and the fixes only work
  together (`overlay20UpdateObjectResource`, `func_overlay_070_F00000D8`).
- A named common-subexpression carrier can consume an otherwise invisible
  ugen temporary even when optimization leaves the same instruction shape.
  In an exact initializer, spelling a just-written pointer chain as
  `array[0] = call(); array[1] = array[0] + size; end = array[1] + size;`
  instead of carrying the call and first addition through locals aligned the
  later integer temporary ring without changing the frame or relocation
  surface. Use this only when no intervening call, volatile access, or
  aliasing write can change the read-back values; require exact configured
  code and linked-ROM proof. Evidence: the exact `func_8004E8E0` closure in
  `docs/resident.md`, 2026-09-08.
- The same read-back carries on a plain global scalar, and it is the cheapest
  ring lever there is on an exact-size plateau. Spelling a byte store as
  `gByte = (u8)gWord;` one line below `gWord = value;` instead of reusing the
  local that produced `value` advances ugen's temp counter by one and emits
  nothing: the load is numbered, then forwarded from the store above it. On
  `overlay57UpdateModeState` the pop landed once in each of two dispatch arms,
  taking the function's ring-temp count from 59 to 61 and its residual from 74
  masked words to 21 at delta 0, after the four pop families that buy a pop with
  an instruction (redundant mask, field read through a local, index scaled
  twice, truncation at the store) had each been measured and each cost width.
  Same preconditions as the array form: no call, volatile access, or aliasing
  write between the store and the read. Read the pop back out of `cc -S`, which
  shows ugen's numbering before as1 schedules. The upstream workbench field
  guide records the array and status-byte instances but not the global-scalar
  one; that generalisation is worth sending on.
- A known-zero byte read can survive as allocator state after its value folds
  away. In an exact release routine, writing zero to a status byte and then
  assigning `status | 1` in the next conditional emitted the same constant
  value as literal `1`, but kept one otherwise invisible UGEN temp-ring pop
  and selected the unsigned OR-immediate form. That one pop aligned the whole
  following flag chain. Apply this only when the store-to-read path has no
  intervening call or aliasing write and the zero value is proved; verify the
  configured words, relocation identities, linked range, and full ROM.
  Evidence: Overlay 47's exact release routine, 2026-09-08.
- A block-local clamp carrier can restore stack-home placement before a later
  narrowing conversion fixes a temp-ring phase. In an exact paired result, a
  scoped word local used for the clamp value moved four existing homes without
  changing frame size, while placing a proved in-range `s16` conversion inside
  the following negate rotated seven terminal temporary registers without
  adding an instruction. Treat these as separate levers: use the carrier only
  for a real value with the same lifetime, and use the inner conversion only
  after proving that every reachable operand already fits the narrow type.
  Out-of-range or otherwise implementation-dependent conversions are not
  interchangeable. Require exact configured code, relocation identities,
  linked bytes, and full ROM. Evidence: the exact
  `overlay94UpdateController` promotion.
- Hoisted loop-invariant addresses are materialized in ugen's birth order,
  so a bound kept in a local born before the count global is issued first;
  spelling the bound inline in the loop test hoists it after the count
  (`func_overlay_014_F0000000_186F8D8`).
- An existing nonvolatile cursor load can affect a temporary-register residual
  through source order alone. Moving the once-only load between two independent
  coordinate assignments made stock full-TU IDO output exact without changing
  command order or stores. For this symptom, consider moving the existing load
  across pure local or by-value parameter assignments; preserve its guard and
  reject crossings involving aliasing writes, calls, volatile access, or escaped
  locals. Keep arithmetic and its defined-input domain unchanged, including
  signed-overflow limits; do not add a read or broaden when it executes. This
  proves source-order sensitivity, not a new internal ugen phase mechanism.
  Require unchanged declarations and exact ordinary code, relocation identities,
  linked bytes, and full ROM. Evidence: the exact resident clear-buffer closure
  in `docs/resident.md`.
- Declaration order can determine stack-home order for call-crossing locals.
  When the operation sequence is exact but spill offsets differ, reorder
  semantically independent declarations before inventing extra state.
  Address-taken local arrays and structs can retain the same
  declaration-relative placement even when preceding scalar and cursor locals
  are colored into registers, so moving an existing aggregate past those
  declarations can change only its frame offset. Producer traces can also show
  that lexical pointer aliases receive automatic homes despite later coloring;
  removing aliases that merely name direct array accesses may shrink the frame
  and move a surviving address-taken scalar to its retail home. Apply that
  narrower lever only when the direct accesses preserve evaluation and update
  order. Evidence: the exact resident collision-handler cohort in
  `docs/resident.md`, plus the exact tile-command builder, spawn-entry
  initializer, and Overlay 98 accepted-entry collector in `docs/overlays.md`.
- Expression association, signedness, and width affect IDO's internal values
  even when C semantics appear equivalent. Preserve the ABI and inferred data
  model; use typed rewrites as a diagnosed lever, not an arbitrary score nudge.
  Evidence: the exact animation, save, and collision cohorts in
  `docs/resident.md`.
- Reusing a word-sized parameter for a value that is narrowed only at its
  eventual byte store can preserve IDO's full-width register web and compact
  frame. Introducing a separate byte local may instead truncate at entry and
  create a byte-sized stack home. Apply this only when every intervening use
  has the same proven value domain and the callee ABI accepts the narrowing;
  reject it unless code, relocations, and linked bytes remain exact. Evidence:
  Overlay 84's exact current-resource activator in `docs/overlays.md`.
- A merged TU or a shared placeholder prototype can change caller code while a
  standalone callee looks exact. Treat symbol binding, visible prototypes, and
  TU ownership as part of the compiler input. Evidence: the merged-TU blocker
  class in `docs/matching-triage.md`.
- An ignored integer return is still part of IDO's allocation input. A caller
  can retain a small register-selection mismatch when its declaration says
  `void` but the callee returns an integer; restoring the authenticated return
  type can close that mismatch without consuming the result. Review the
  winning translation unit's declarations as well as its function body when
  transferring a search result. This is an ABI correction, not permission to
  vary return types for a better score: prove the callee's return behavior and
  check every affected caller, configured object, relocation and linked ROM.
  Evidence: Overlay 4's exact object-motion update and its scoped header.
- Under O32, a single 64-bit integer argument occupies an aligned `a0`/`a1`
  pair. IDO materializes the two halves of a constant zero independently, so a
  target with two adjacent argument-register zero loads can indicate one
  `u64`/`OSTime` parameter rather than two scalar parameters. Use relocation or
  callee metadata to prove that ABI before changing a prototype; equal register
  contents alone are insufficient. Evidence: Overlay 18's exact startup loader
  in `docs/overlays.md`.
- An algebraically zero integer read can be a caller-saved coloring lever. On
  a path that already returns zero, spelling the result as `value * 0` kept the
  value's web live through IDO's allocation decision while still folding to a
  zero return instruction; a two-web forced-color probe identified the cause,
  and the same stock source then matched without a force. This is specific to
  the measured `-O2 -mips2` basin: use it only when zero is semantically
  required, and reject it if instruction count, relocations, or linked bytes
  move. Evidence: Overlay 1's exact value-cache manager in `docs/overlays.md`.
- Expression shape can affect allocation even when the changed subexpression
  is constant. A bounded search paired unsigned XOR-zero around a complete
  constant RGB packing group with an equivalent logical-negation zero test;
  together they produced exact stock full-TU output. Unlike the value-read
  example above, this XOR introduces no new live input. When transferring the
  retained expression, preserve unsigned types, side effects, evaluation count,
  and the one-line display-list macro block grouping. This proves the paired
  result, not either change's individual cause, a precise compiler phase, or
  recovered original source. Do not generalize to undefined expressions or
  altered side effects; require exact owned bytes, relocation identities and
  linked ROM. Evidence: the exact cone-drawing closure in `docs/resident.md`.
- **An algebraic reassignment can change allocation at zero emitted width.**
  When ordinary self-copies and discarded reads disappear too early, assigning
  an initialized integer cursor through unsigned OR-zero or XOR-zero can retain
  extra weighted references and change the allocator's recorded divisor. IDO
  can then remove the operation before the final instruction stream. Compare
  fresh allocation records for the actual source, not just a forced colour:
  addition by zero need not behave like bitwise OR by zero. The exact
  [strip-drawing closure](matching-triage-handoffs/overlay17DrawStrip.md)
  demonstrates this after an earlier closure incorrectly excluded a zero-width
  spelling. Transfer tests on the
  [overlay renderer](matching-triage-handoffs/func_overlay_071_F0000870_18CA390.md)
  and [object loader](matching-triage-handoffs/func_80006534.md) did not improve
  their residuals, so this is a measured lever, not a general ratio-setting
  recipe. A pointer round trip additionally requires the supported target's
  pointer-width and integer representation guarantees. Preserve evaluation and
  memory effects, disclose an inert retained spelling, queue naturalization,
  and require stock bytes, relocation identities and linked-ROM proof.
- A source copy can decouple load order from FP coloring when two equal-cost
  webs tie. Load the first value through the local that must receive the first
  color, copy it to its lasting local, then overwrite the first local with the
  second value; IDO can coalesce the copy away while preserving the opposite
  load and color orders. This applies only when the copy and overwrite are
  semantically exact and the stock object emits no move or spill; reject the
  shape unless instruction count, relocations, linked bytes, and the full ROM
  remain exact. Evidence: Overlay 8's exact scale-output body in
  `docs/overlays.md`.
- When two independent initializations need reversed caller-saved colors but
  separate statements let the scheduler split a relocated address pair,
  preserve their evaluation order in one comma expression. IDO can retain the
  desired web-formation chronology while scheduling the address pair together;
  separate-statement orders may independently fix either coloring or schedule
  without fixing both. Apply this only to semantically independent
  initializations, and reject it unless instruction count, relocation identity,
  linked bytes, and the full ROM all remain exact. Evidence: the exact camera
  override search in `docs/resident.md`.
- For a commutative address calculation, source operand order can select which
  producer receives each block-local temporary without changing the final
  `addu`. When only the base load and scaled-index shift exchange ring
  registers, spell the proven-equal expression as integer-plus-pointer or
  pointer-plus-integer to match their evaluation order; reject the lever if any
  other word, relocation, or linked byte moves. Evidence: the exact Huffman
  table builder in `docs/resident.md`.
- For a commutative add whose other operand is an ILOD off a forwarded temp
  (a value stored and re-read so ugen keeps it in a ring register across a
  branch), uopt canonicalises the ILOD first: `uadd(cvt(ILOD(temp)), isvar)`.
  The same add against an isvar operand canonicalises the symbol first, which
  is the object-first order the plain, non-nested form already emits. Source
  operand order, reassociation, and casts on the ILOD are byte-flat; the
  lever is to bind the nested load to an existing isvar that already owns a
  frame home. A fresh local takes a new home and moves every displacement.
  Reusing one symbol at two nested sites reconnects as a single web and can
  re-steal an earlier numbering tie. Confirm with `uopt -Wo,-zdbug:2`
  (`uoptlist`) rather than by spelling the add. Evidence: the exact
  `func_8000590C` nested fixups.
- IDO can normalize both orders of pointer-plus-byte-offset addition to the
  same temporary demand order. If the final address addition matches but its
  base-load and offset-shift producers exchange temporaries, an unsigned
  address sum can retain a different operand order: cast the base to the
  target's pointer-width unsigned integer, add the byte offset, then cast
  back. This is a measured source lever, not proof of allocator-pass ownership.
  Apply it only to established raw addresses on the 32-bit N64 ABI, with the
  same accesses and address result; it is not a portable pointer-arithmetic
  rewrite. Require exact instruction words, relocation identities, linked
  bytes, and the full ROM. Evidence: the runtime overlay unload-reference
  patcher in `docs/overlays.md`.
- When a computed magnitude is immediately tested and then inverted, assigning
  it through the eventual scale local before the test can preserve IDO's
  floating-point carrier web. Testing one local and assigning the reciprocal to
  another may split otherwise equivalent live ranges and recolor every
  downstream operation. Use this only when both locals represent exactly the
  same value at the test and no path observes the intermediate separately;
  require exact code, relocation identities, linked bytes, and full ROM.
  Evidence: Overlay 1's exact motion-point resolver in `docs/overlays.md`.
- In a floating-point coefficient sum, a mathematically equivalent term order
  can change IDO's expression tree and the temporary schedule of later
  polynomial evaluation. When a residual is confined to that evaluation,
  preserve the target's inferred association by moving the proved independent
  leading term to the end of the source sum; also test operand order on a
  commutative scalar multiply. For an independent difference of products,
  spelling `a*b-c*d` as `-(c*d)+(a*b)` can rotate one term without changing
  the result; an explicit `(*pointer).field` may also preserve a remaining
  operand-order choice that the arrow spelling does not. These levers apply
  only when the arithmetic domain and evaluation order are proved
  interchangeable, and they must be rejected unless the configured object,
  relocations, linked range, and full ROM are exact. Evidence: Overlay 41's
  exact curve sampler and Overlay 22's exact plane resolver in
  `docs/overlays.md`.
- When otherwise exact single-precision code differs only in a multiply's
  operand order, an identity product on an already-computed operand,
  `(x * 1.0f) * y`, can change IDO's emitted operand order without adding an
  instruction. This expression-shape effect is measured; the optimization
  pass responsible is untraced. Treat it as a narrow source-spelling lever,
  not a general floating-point equivalence rule: do not duplicate side effects
  or volatile reads, assume exceptional-value behavior, or substitute a double
  literal. Reject the form unless instruction count, frame, relocation
  identities, linked bytes, and the full ROM remain exact. Evidence: the exact
  Overlay 29 point-projection TU, `src/overlays/o029/overlay29ProjectPoint.c`.
- A display-list macro invoked with a postincremented cursor keeps its local
  command pointer, cursor advance, and field writes on the invocation's source
  line. IDO can then schedule the cursor advance before the writes and choose a
  different order for the command-word stores and constant materializations.
  Hand-expanding those operations across separate lines preserved the same
  instruction multiset but not the schedule; delaying both cursor advances let
  IDO coalesce two commands and removed three instructions. Use the native macro
  only with its complete header context (`_SHIFTL` must be a macro, not an
  undeclared call), and reject it unless command semantics, relocations, linked
  bytes, and the full ROM remain exact. A donor adaptation also showed why
  reversing the C stores to copy the target's final store order can be the
  wrong lever: it changed the address/constant temporary order. Retaining the
  donor's expression order with the two stores on one physical line produced
  the exact schedule. Statement order and line grouping must therefore be
  reviewed together; this does not identify the responsible compiler pass or
  justify arbitrary line reflow elsewhere. Evidence: the exact resident
  `func_80034920` display-list reset and the
  [snow renderer donor re-derivation](matching-triage-handoffs/snow_render.md).
- An inlined mask expression can preserve instruction count, frame and opcode
  order while changing register operands throughout later display-list code.
  Reusing an existing integer local for successive width and height masks,
  as established by a permitted donor, removed that register residual without
  adding instructions. The measured cause is the source carrier; no allocator
  pass or temporary-pool mechanism was established by a trace. Apply this only
  to proved disjoint lifetimes, with unchanged expression types, memory
  accesses and arithmetic. It does not justify dead locals or an arbitrary
  declaration search. Evidence: the exact resident track draw coordinator in
  `docs/resident.md`, including its configured full-TU donor comparison and
  linked/runtime relocation proof.
- Across consecutive clear loops, an unchanged parsed C tree can still emit a
  different address-materialization schedule when physical line boundaries
  change. A relevant span may begin inside one loop, cross its closing brace,
  condition and intervening statements, and end at another loop's opening
  brace. Preserve the complete span through candidate emission, not merely
  adjacent assignments; retain every expression, statement and scope as
  searchable C. Exact lexical/AST correspondence can establish the boundaries,
  but only a fresh compile establishes source-carriage fidelity. The compiler
  pass responsible for this measured sensitivity remains untraced. Ordinary
  local pointer-assignment spellings can also affect the result; when spelling
  and grouping change together, do not attribute the exact result to either
  lever alone. Require unchanged semantics, exact owned bytes and relocation
  identities, linked-range and full-ROM proofs. Evidence: the matched initialization
  routine in `src/main/anim.c`.
- Typing an interleaved two-word cache as an array of structs can make IDO
  strength-reduce an index loop into advancing entry and byte-offset pointers;
  differently typed sentinel fields can also materialize separate copies of
  the same `-1`. Keeping the externally proved storage as a flat word array and
  spelling its ID/pointer slots as `(i << 1)` and `(i << 1) + 1` preserves the
  index, emits the per-iteration `i << 3`, reuses that byte offset for cleanup,
  and can share one sentinel carrier. Apply this only when allocation, element
  width, and both slot meanings are independently authenticated; require exact
  frame, relocations, linked bytes, and full ROM. Evidence: the exact resident
  `func_800359D4` sprite release.
- A canceling pointer increment/decrement between two groups of stores can be
  a zero-instruction allocation lever. IDO eliminates the net-zero arithmetic
  but retains enough of the pointer lifetime split to change a two-web
  caller-saved coloring; a bounded permuter found this after natural source
  already had the exact opcode, frame, and relocation shapes. Use the form
  only when the intermediate pointer is valid within the independently proved
  array and no alias can observe it, disclose it as inert, and keep a cleanup
  task for a natural equivalent. Require the ordinary project object, linked
  range, and full ROM to remain exact. Evidence: the exact resident
  `func_80035ADC` sprite-frame builder.

- Preserve authenticated array identity when reconstructing entry loops.
  Under unchanged configured full-TU flags, direct integer indexing into a
  bounded global entry array restored source-pointer hoisting and four-entry
  unrolling that an advancing destination pointer did not produce. In the
  same reconstruction, direct indexed digit stores preserved shared quotient
  and remainder calculation without staging extra locals; mixed stores
  through a less specific cursor had duplicated division work. These paired
  builds prove sensitivity to the source's object/index representation. An
  alias-analysis explanation is an inference, not a traced compiler cause.
  Check actual element widths, array bounds, and read/write order before
  applying the lever; do not change storage layout or invent a larger object.
  Compare the affected loop or call interval as well as whole-function size,
  because duplicated arithmetic can cancel a missing loop elsewhere.
  Equal size or an equal relocation count still does not establish a match.
  Evidence: the controlled reconstruction recorded in the
  [overlay 52 HUD handoff](matching-triage-handoffs/func_overlay_052_F000063C_189ACAC.md).

- **A remainder-plus-4x copy loop in the ROM is the unroller's output, not a
  source shape.** Spelling the remainder test and the four-at-a-time body in C
  cannot reach that CFG: the unroller then unrolls the already-unrolled form
  and the function grows by hundreds of bytes. Spell `for (i = 0; i < n; i++)`
  over the logical element. The unroller emits the remainder loop, the 4x
  body, the bypass of the leftover test on the no-remainder path, and the
  strength-reduced zero-shift preheader. A per-TU `-Wo,-loopunroll,0` that
  only the unmatched functions need is a claim about those candidates, not
  about the TU. Limits: do not reintroduce the override to paper over a
  hand-unrolled candidate; `while` and `do` with an explicit `n > 0` guard
  are not interchangeable with `for` here (they added eight words at delta
  0). Evidence: the resident mode-record rank copy in `src/main/objects.c`.
- **A byte copied onto a signed field is a signed load.** At delta 0, ten
  structural words on an otherwise exact copy loop were an unsigned source
  field against the target's signed load. Declaring the source `s8` closed
  them with no other edit. Limits: this is the load's signedness, not the
  loop's; it does not license a cast at the store as a substitute. Evidence:
  the same resident rank copy.
- A large unrolled tail can depend on the source loop's control form even
  when the configured TU flags stay fixed. Paired full-TU builds of a counted
  `for` sort and its reconstructed guarded post-decrement form recovered the
  compact loop; an earlier no-unroll diagnostic independently localized the
  same expansion. Reconstruct whether each branch tests the value before or
  after decrement, including zero-count entry guards, from the target before
  changing C. This proves source-control sensitivity; identifying the exact
  unroller pass remains an inference without a phase trace. Do not add guards
  merely to change compilation, assume all post-decrement loops avoid
  unrolling, or treat equal size as a match. Preserve zero/one-iteration
  behavior and the order of paired record writes. Evidence: the controlled
  [height-query reconstruction](matching-triage-handoffs/func_8001357C.md).

- A decompiler-hoisted invariant assignment does not prove its original
  source placement. In a paired full-TU sort build, moving the bound from
  inside each outer pass to before the loop suppressed unrolling that the
  target required, under unchanged configured flags. Reconstruct the branch
  values and compare the affected loop before adopting the draft's placement;
  the compiler pass responsible remains untraced. Compact and unrolled
  targets each require their own control-flow evidence.
  Evidence: the [collision-height audit](matching-triage-handoffs/func_8001398C.md).

- A narrow field load does not establish the original width of a local or
  loop counter. In a paired full-TU build, narrowing signed-halfword endpoints
  and their bounded counter introduced shift/extension instructions even
  though the value interval fit the type. The resulting near-equal function
  size came from additional conversion code. Keep
  field widths and counter widths as separate hypotheses, inspect the added
  instructions, and do not treat a range proof as a promise that IDO removes
  conversions. Evidence: the [nearest-hit audit](matching-triage-handoffs/func_8001291C.md).

- uopt emits every plain pre-guard statement before every hoisted loop
  preheader initialiser, whatever their order in source. That makes statement
  placement a single choice with two coupled consequences, not two knobs. A
  carrier initialised before a zero-trip guard dominates the guard's skip edge,
  so a later re-initialisation of the same carrier is redundant there and IDO
  retargets the guard's branch past it; moving the initialiser inside the guard
  makes the re-initialisation live and the branch target exact, but the
  initialiser is now a preheader and is emitted after any plain statement that
  precedes the guard. Where the target wants the carrier first, no arrangement
  of the same statements delivers both, and the arrangements that try to put
  the second value in the preheader too leave it uninitialised on the skip path
  and cost instructions. Recognise the pattern from a one-word branch-target
  residual on a guarded scan. Evidence:
  [the particle pool scan](matching-triage-handoffs/func_8003E8D8.md).

- Mentioning a value earlier in source earns it an earlier colour only if the
  earlier mention survives. A comma expression or any other dead first mention
  is eliminated before web numbering and moves nothing, and on one unit a plain
  zero initialiser of an otherwise unused local was likewise eliminated before
  ring numbering and did not advance the ring. The reserve-a-colour lever is
  therefore not general: confirm on the unit in hand that the reserving
  construct survives, because both outcomes have now been measured. Evidence:
  [the model-release loop](matching-triage-handoffs/levelFreeAll.md) and
  [the CPU dump helper](matching-triage-handoffs/func_80045BBC.md).

- Ring order inside one expression is the order of the surviving definitions,
  which means an expression with three operands offers only as many ring
  orders as it has evaluation orders. Where the target's order is not one of
  them, no spelling of that expression reaches it. Splitting the mask into a
  *fresh* local does not help: uopt forward-substitutes a single-assignment
  temporary straight back into the address. Mutating the *existing* carrier
  that produced the value -- `x &= M` on the loaded entry, then a base-first
  add of the table pointer -- is a surviving evaluation in front of that
  address, and ugen then emits mask, table, scale. The carrier must be 32-bit
  or the assignment is a narrowing chain. Evidence:
  [the model-release loop](matching-triage-handoffs/levelFreeAll.md).
- **A chained assignment leaves the pair one web; two full expressions give
  each its own colour.** Writing `a = b = X` makes the later use of `a` read
  back whatever carrier `b` ended up in, and it also shifts every colour
  assigned after the pair, because only one of the two names spends a colour.
  Spelling each component as its own complete expression -- the same expression
  twice, not a copy -- gives each name a pool colour and puts the next variable
  after both in colour order. Measured on `overlay62Update`, where the chained
  form left seven pure register differences that survived 720 declaration
  orders and a 1,920-cell hint/qualifier lattice, and the split form was exact.
  The lever composes with the arithmetic spelling and neither works alone, so a
  single-edit accept rule rejects both (the L88 composition rule).

- **Work locals that are dead on an early-exit path belong after the exit.**
  IDO emits them where the source puts them, and their position decides what
  the join block starts with, which in turn decides what fills a branch-likely
  delay slot above it (see the delay-slot entry below). Order *inside* the
  moved group is then load-bearing: on `overlay75UpdateMovingObject` exactly
  two of the 120 orders of the five moved statements are exact and the rest
  cost up to 273 words, which is why earlier "move the initialisers" attempts
  read as a flat regression. Check the exit block actually ignores the values
  before moving them; there it reads only a cached pointer.
- **A caller-saved colour is won by making the lower colour unavailable, and a
  tested dead expression is how you do it.** `globalcolor` walks a web's
  admissible colours in ascending order and keeps the first *strict* minimum,
  so where two caller-saved registers both cost 0.0 the lower-numbered one
  always wins: the colour ladder is v0, v1, a0, a1, a2, a3, t0..t5, then
  s0..s8. The instrumented `p1dec` row's `forbidden0` decodes as *bit
  `31 - colour`*, and it holds exactly the colours of interfering coloured
  webs plus the registers pinned by calls or parameters the web is live
  across. So a residual that is one caller-saved register too low is never a
  spelling question -- no rearrangement of the statements that already exist
  can add a colour -- and the only lever is one more interfering caller-saved
  web that emits nothing.

  A dead expression **used in a condition** supplies it. `if (start << 2);`
  gives uopt a register temporary with the expression's live range; the
  temporary takes the lower colour and the value you wanted moves up one.
  Three constraints were each measured, and each one alone is fatal:

  * **Two references, not one.** A single mention is folded before web
    numbering and reserves nothing -- which is why every earlier single-site
    dead-store and comma-expression probe on these functions read as flat.
  * **The value must be tested.** `if (E);` and `if (!(E));` work; a bare
    `E;` statement, `(void)(E);`, and `while (0) { E; }` are all discarded
    first. `E && E` inside one `if` costs branches, and repeating the
    expression inside one statement is CSE'd back to one reference.
  * **It must out-rank the web you are moving.** uopt colours by descending
    `save = references / bucket(references + spanning statements)`, ties
    broken by ascending web index, and the buckets are 2 for a raw count of
    3..5, 3 for 6..9, 4 for 10..13. A reservation placed a couple of
    statements too far away loses a bucket, drops from 1.5 to 1.0, and
    colours *behind* its target. Put the second reference on the target web's
    own last use.

  And the temporary is not free: it takes a `spilltemps` slot, so the frame
  is `round8(4 * declared locals + 4 * temporaries)` plus the fixed blocks.
  Twelve declared scalars plus one temporary rounds up a step; carrying one
  value in an already-declared local instead of its own (here the
  `func_8000572C` result in `temp_s0` rather than a separate `temp_v0`) buys
  the step back. Measured on `func_80004454`, `func_8000471C` and
  `func_80009AA8` in `src/main/objects.c`, all three of which had survived
  132 declaration permutations, ten dead-store spellings, the L87
  `field == (x ^ 0)` trick on both operands, six selection restructurings and
  a flag lattice at three register words each; all three are byte-identical
  with this edit, and the translation unit's other functions do not move.

- **Before adding a web, try moving a symbol boundary: it reorders `globalcolor`
  for free.** The reservation lever above adds a web to make a lower colour
  unavailable. On a procedure that cannot afford one -- or where the residual is
  a whole ladder rather than a single register -- the same reordering is bought
  by changing which *symbol* a value's references belong to, at zero width. uopt
  colours by descending `save = totalsave / nocs`; `totalsave` is the web's
  reference count with references inside a loop weighted x10, and `nocs` grows
  with the web's span. Both terms are moved by declaration scope alone:

  * **Merging raises save.** A variable declared once per block in two arms of a
    dispatch is two webs; declared once at function scope it is one, with double
    the references. On `overlay57UpdateModeState` the marked-entry pointer went
    from 42/3 = 14 to 84/4 = 21 and jumped four places up the colouring order.
  * **Naming splits, and splitting lowers save.** One cfe temporary serves the
    same construct in every arm, so a compiler temporary is usually the *longest*
    web in the region. Giving it an explicit per-arm name halves it: the same
    function's dead post-decrement copy went from 44/2 = 22 to 22/2 = 11 and
    fell three places.

  Neither edit alone helped -- each was a 31-word regression, the L88
  composition rule again -- and together they put the three loop webs in exactly
  the target's order (21 > 17 > 11) and closed 16 words. Read the saves out of
  the traced `p1dec` rows first and solve for the order you need; do not sweep
  spellings. Evidence:
  [the mode-state dispatch](matching-triage-handoffs/overlay57UpdateModeState.md).

- **A web whose span reaches a call result has v0 struck from its candidate list
  entirely, so "one register too low" can be a span question, not a colour
  one.** The instrumented `p1cost` rows list only admissible colours. For a web
  live across a call whose result is used they begin at colour 2: v0 is not
  forbidden by an interfering coloured web, it is simply not offered, and
  `CDX_FORCE ...=c1` on it is declined silently. A residual where the target
  reads v0 and the candidate reads anything else is then unreachable by any
  colouring lever, and the fix is to *shorten* the web until it no longer spans
  the call. Shortening is the mirror of the naming lever above: on
  `func_overlay_086_F0000474_18D22AC` two dereference sites in different switch
  arms shared one cfe temporary, making a six-reference web that spanned both;
  carrying one site in an already-declared local split it, and the other site
  became a one-block web that regained v0, took it, and pulled a ten-row
  caller-saved family down a step with it -- 51 words to 30 at delta 0, with no
  new declaration and no frame move. Check `p1cost` before assuming a colour is
  reachable. Evidence:
  [the overlay 86 update](matching-triage-handoffs/func_overlay_086_F0000474_18D22AC.md).

- **uopt re-materialises a cheap masked value instead of keeping a hoisted local,
  so you cannot buy a web by naming one.** Hoisting `x & M` into a local used
  four times inside a loop leaves the loop body byte-identical -- the mask is
  recomputed at each use and no web appears -- and the only effect is the frame
  cell the declaration costs. The same holds for a single-use local in an
  address expression: uopt forward-substitutes it back into the expression
  across basic-block boundaries, through an `if (1) { }` or `do { } while (0)`
  region, and out of a condition's comma. Twelve hoisting forms on
  `levelFreeAll` all collapse to one of the two orders the two literal
  spellings already produce. If a value must become a web, it has to be
  expensive enough that uopt will not recompute it.

- **Check the procedure for a register-pressure cliff before spending a day on
  a reservation.** The dead-expression lever above adds a web, and a procedure
  whose marginal web is already at the edge of its colour pays a fixed toll for
  any web at all -- wherever it is put and whatever it computes. On `levelInit`
  (`src/main/level.c`) one `if (E);` is byte-identical and **any second one
  costs exactly +120 bytes and 30 words**: nine expressions x five position
  sets inside the target loop, two and three references in a single basic
  block, and a control placed at the top of the function on a parameter, 54
  cells and one number. The traced `globalcolor` names the toll: the marginal
  web (save 1.18, 45 interferers) holds the last callee-saved colour with
  `decision=color` in the base and flips to `decision=split` with any extra web
  present, and the 30 words are its reloads. The cheap pre-check is one
  compile: add a single throwaway web and diff the size. If it is not free, the
  whole reservation family is closed for that procedure and the only route to a
  caller-saved colour is to *remove* a web instead.

- **A web that `globalcolor` splits has no colour, so no force sweep can reach
  it.** uopt colours a web only when its best cost is strictly below its
  `totalsave`; otherwise the record reads `decision=split` and the value falls
  through to ugen's ring. A residual that is one register wrong is therefore
  worth one `CDX_DETAIL_WEB` read before any sweep: if the contested value's
  web is split, forcing every *other* web to every colour is guaranteed to find
  nothing, which is exactly what two passes on `func_8003F154`
  (`src/main/particles.c`) spent 100 forces discovering. There the magnitude
  test's `0.0f` is a float constant web with `totalsave 1.0` against `cost 3.0`
  at every available caller-saved colour, so it is split; reaching the target's
  register needs the cost below the save -- four references to the constant, or
  a live range crossing no call -- *and* the colour freed by an interfering
  web, two conditions rather than one.

- **The fp pool ladder is `$f0, $f2, $f12, $f14, $f16, $f18`, and ugen's fp
  ring is `$f4` to `$f10` plus every ladder register no web claims.** Measured
  on `func_8003F154` by forcing each fp web to each of the six colours (24..29)
  and reading which register moved, and cross-checked against the instrumented
  `ugen` free list, which rotates five wide there because `c25` (`$f2`) is
  taken. The consequence is that "the target has this value on the ring, the
  candidate has it on a pool colour" is a *counting* statement: the target
  spends one fewer fp web in that region. Reading the ring membership is the
  cheapest probe in an fp residual -- `DKWB_UGEN_TRACE=1` filtered to
  `ALLOC_FP_RESULT` for the procedure's ordinal answers "did this edit free a
  pool colour" in seconds, without scoring anything.

- **A uopt region boundary moves the fp pool/ring split, not just the integer
  ring phase.** `do { ... } while (0)` opened at the sqrt statement in
  `func_8003F154` frees `$f16` from the pool and widens the fp ring from five
  to six, at delta 0; four other placements do the same. So the region-boundary
  lever is worth trying on any residual described as fp-pool-versus-ring, with
  the caveat measured there: 21 placements across one block moved the split
  three ways and none of them freed the register the target actually wanted.

- **A compiler temporary's stack slot is a second colouring in web order, not
  a priority.** After `globalcolor`, uopt's `spilltemps` walks its register
  temporaries (induction pointers, common-subexpression values, call-crossing
  copies -- never declared locals, which already have homes) in web-index
  order and hands each the lowest-numbered existing slot of the same size
  that no *interfering, earlier* temporary holds, else a new slot laid just
  below cfe's local block (the declared locals, then cfe's own call-result
  temporaries). Every temporary takes a slot whether or not it ever touches
  memory, so a coloured induction pointer still blocks the slot of the
  spilled one that shares its loop. Web index is creation order, which is the
  first textual encounter of the expression anywhere in the function, and the
  loop-exit test goes to the last-created induction pointer, so reference
  order alone cannot move a slot and the exit variable independently.
  Measured on `func_overlay_060_F0000334_18BA10C`, whose last four words were
  one name-table pointer homed at 92(sp) against the target's 96(sp) with
  every instruction already identical: the pool started four bytes low
  because a nested `u16` call result used as an argument
  (`frontGetSfxVolume()`) gave cfe a halfword temporary below the locals, and
  the float that had to stay at 80(sp) needed one more pooled temporary
  ahead of it, which reading `gOverlay60Data0C8[i]` directly (a CSE
  temporary) rather than through a declared `object` (a homed local, never
  pooled) supplies. Neither edit changes an emitted instruction. The CDX
  `p1dec` trace does not show slot choice; a print in the recompiled uopt's
  `f_spilltemps` (per temporary: web, blockers, chosen slot, new offset) and
  `f_gettemp` reads it directly, and the cfe `Uvreg` records in a `-K` Ucode
  dump name the front-end temporaries and their sizes.

- **The pooled slot is an arithmetic function of a countable census, so
  calibrate it instead of searching it.** Cell `k` sits at
  `frame_top - 4*(k + 1)` and the frame is `round8(4 * cells)`, which makes the
  whole thing readable off two builds. Add one unused declared local: if every
  home moves down exactly one slot and the frame grows, each declaration costs
  one cell and the census is the lever. Measured twice on the same day:
  `overlay1InterpolatePath` (frame 0x68, ten declarations, adding an unused one
  moves the frame to 0x70 and every home down a slot) and `func_8005716C`
  (a `volatile` local's home tracks the declaration count exactly, 8 -> frame
  0x20, 9 -> 0x28). Once the arithmetic is fixed, the target's home names the
  cell index it wants and you edit the census to hit it, rather than permuting.
  On `overlay1InterpolatePath` the answer needed **two** edits (L88): defining
  the spilled integer earlier swaps it with the neighbouring cell, and dropping
  one declared local lifts the pair by four bytes; each alone was a regression
  and the pair was exact. Corollary worth knowing before you start: *all* 196
  legal statement orders of that function's setup reached only two adjacent
  cells, and all 90 single-move declaration permutations were flat -- a cell
  residual that reads flat under reordering is a census question, not an
  ordering one.

- **`volatile` on a local is what forces the reload to be named, and a name is
  a pool colour.** Where a candidate collapses a chain of intermediate values
  onto one FP register and the target spends a fresh ring temporary at each
  step, look at the qualifier before the expression tree. A `volatile f32`
  round trip cannot be read twice in one expression without emitting two loads,
  so the source has to name the reload -- and the named local takes a pool
  colour that then carries the negation, the doubling and the product too.
  Dropping `volatile` keeps the memory round trip whenever the pool is already
  full (uopt simply cannot colour the extra symbol, which the instrumented
  `p2dec` shows as a `decision=no-color` with a nonzero `bestcost`), while
  letting `x + x` load once and hand the sum to ugen ring temporaries.
  On `func_8005716C` that made all 80 words agree, registers included.
  The cost is two extra pooled cells, at any declaration count -- so the same
  edit that fixes the registers moves the frame, and the two have to be solved
  together.

- **Scope limit on the entry above: volatility does not reach an
  address-materialisation residual.** A lane reported the volatile/reload lever
  as "falsified"; it is not falsified as written, but its framing does not
  extend past FP reload naming, and reaching for it on the wrong residual class
  costs a full sweep. On `func_80002134` (`src/main/audiomgr.c`, 9 words) the
  residual is an **address-CSE** decision: the candidate materialises the
  address once and reuses it across the branch, while the target recomputes the
  `%hi`/`%lo` pair at both accesses. 28 forms held flat at 9. The proof that
  volatility is not the axis: removing `volatile` **regresses to 11 and still
  materialises the address**. So a residual whose signature is a repeated
  `%hi`/`%lo` pair is a CSE question, not a qualifier question, and the entry
  above has nothing to say about it.

- **Re-gate the `func_8005716C` claim above before relying on it.** That entry
  cites `func_8005716C` as the case where the edit "made all 80 words agree,
  registers included". The function currently measures **3 differing words**
  (improved from 5 on 2026-09-10 by an unrelated closed-form frame argument:
  with N frame cells the frame is `align8(4N)` and the last cell's home is
  `align8(4N) - 4N`, so a home at `4(sp)` requires N odd). Either the original
  claim covered a region rather than the whole function, or it has gone stale
  under later adoptions. It is recorded here as a discrepancy rather than a
  correction because nothing in this session re-derived the original
  measurement -- do that before treating the 80-word agreement as current.

- **A declined `CDX_FORCE` is a real result: it retires the reordering space.**
  `CDX_FORCE=p2:w<n>=c<colour>` refused at both the `dec` and `color` sites,
  with the wanted colour inside `forbidden0`, says the colour is unavailable
  through genuine interference and not through a priority choice. No statement
  order, declaration order or spelling that leaves the interfering web in place
  can reach the target, so stop sweeping them and go after the interfering web
  itself. Read its identity from the `intf` rows under `CDX_DETAIL_WEB`:
  on `overlay20RemoveEntry` the compaction limit (web 42) is blocked by an
  *invisible* web 8 that holds v0 and emits no instruction anywhere, and on
  `func_80046BCC` the masked character is its own web, coloured ahead of every
  declared local, which is why forcing the loaded byte back into v0 is refused.
  Both readings cost one instrumented compile each and each retired a lattice
  that had already consumed a full pass.

- **A bitfield read through the word and written through the halfword is
  a `u16` bitfield.** IDO reads a bitfield through a container of the
  declared type's width but stores it through the smallest aligned container
  that covers the field: on `overlay74Update` the target's `lw`, `sll 5`,
  `srl 28`, `sll 7`, `andi 0x780`, `lhu`, `andi 0xF87F`, `or`, `sh` sequence is
  a 4-bit field at bits 26..23 of the word declared as `u16 pad:5; u16
  field:4; u16 rest:7`. The hand-expanded shift-and-mask spelling reproduces
  every word but one -- the operand order of the final OR, which the source
  then fixes and which reversing costs a word elsewhere -- while
  `flags.field |= bit` lets cfe order it, and matches. The `u32`/`s32`
  containers keep the halfword store but cost 12 words; `u8` changes the
  container. A hand-expanded bitfield is therefore a residual signature in
  its own right: when the mask/shift arithmetic matches and one commutative
  operand order does not, declare the bitfield.

- **The caller-saved temporary ring is one counter over the whole procedure,
  and `cc -S` reads it out directly.** ugen hands every expression temporary
  the next register in the fixed cycle `$14,$15,$24,$25,$8,$9,$10,$11,$12,$13`
  (`t6,t7,t8,t9,t0,t1,t2,t3,t4,t5`), advancing once per temporary created, in
  post-uopt emit order and never reusing a name early. A residual that is a
  *uniform rotation* of that cycle from some point onward -- every opcode,
  offset, operand and relocation identical, only `tN` names shifted by a
  constant k -- is therefore not a colouring question at all: the target
  simply created k more temporaries before that point, and the only question
  is which source expression supplies them. Do not sweep declaration or
  statement order for it; the rotation is invariant under both.
  A temporary can cost a ring slot and emit no instruction, which is what
  makes the count invisible in the disassembly: uopt forwards a store to a
  following load of the same field, and as1 coalesces a `move` into the
  instruction that defines it, so both leave a consumed ring position and no
  word. `cc -S` (which the driver produces via ugen's `-l`) prints the
  pre-scheduling listing with these registers already assigned, so the ring
  index of any expression is directly readable and a candidate can be scored
  by *ring position* before it is ever assembled -- much sharper than a
  positional word diff, which reports a pure rotation as dozens of differences.
  Evidence: `func_80019DE8` in `src/main/lights.c`, whose last residual was
  16 words that were exactly a +2 rotation. Spelling the delta as
  `state->valueDelta = state->endValue - state->startValue` instead of
  `arg2 - arg1` -- the two bytes were stored on the two preceding lines, so
  uopt forwards both and neither load survives -- spends exactly the two
  missing ring slots and matched. This is lever 45 used for its *ring cost*
  rather than for a register-to-register copy: the same edit, read through
  the counter instead of through the copy.

- **Two webs defined in one statement cannot be separated by any order, but a
  dead `= 0` ahead of them reorders their colours.** uopt colours pool webs in
  first-surviving-definition order. When the source defines two of them inside
  a single statement -- `doubled = -(... (velocityX * (normalX = normal->x))
  ...)` defines `normalX` and then `doubled` -- their relative order is fixed
  by evaluation order, so permuting statements or declarations moves nothing
  and the residual reads flat. That flatness is the signature, not a dead end.
  Defining the later web *earlier*, with a store uopt deletes but the web
  numbering has already seen (`doubled = 0.0f;` before an unrelated statement),
  swaps the two colours and emits no instruction. Verify the freeness: the
  candidate must keep the same word count and zero opcode mismatches.
  Evidence: `func_8005716C` in `src/main/anim.c`, where the f16/f18 rotation
  had survived 512 operand orders and every declaration order, and this closed
  it (10 differing words to 7). The permuter reached the same effect as an
  uninitialised self-add inside a `do {} while (0)`; that is undefined
  behaviour, and the zero store is its semantics-preserving re-derivation.
- **A value the target computes straight into a callee-saved register, and the
  candidate computes into a caller-saved temporary and copies, is a
  live-range split, and it exchanges every colour after it.** The candidate's
  extra carrier renumbers the saved webs, so two unrelated variables appear
  swapped and the residual looks like an allocator phase. Read the *first*
  instruction that writes the value, not the copies.
  Evidence: `func_80046BCC` in `src/main/diCpu.c` (`andi s2,v0,0xff` against
  `andi v0,v1,0xff` + `move s3,v0`), where that one decision is the whole
  31-word residual.
- **A pointer tested for NULL through a cached local gives the load a copy;
  testing the field directly does not.** Where the target loads a field into a
  register, branches on it, and copies it into a second carrier only on the
  surviving path, spelling the guard as `p->field == NULL` rather than
  `local = p->field; if (local == NULL)` keeps the load in the first argument
  register and gives the copy to the second. The cached form exchanges the two
  argument registers across every later use of both carriers. Note this is the
  *inverse* of the two-carrier field re-read above: the second declared
  pointer is what buys the copy while a size question is open, and what causes
  the exchange once the size is closed. Evidence: `func_8005ABA8` in
  `src/main/models_5B300.c`, worth seven words.
- **Where a flag is set on every path of a branch, writing it once after the
  branch rather than inside each arm changes no instruction but changes as1's
  delay slots.** as1 fills an annulled (`beql`-family) delay slot by
  duplicating the branch target's first instruction and retargeting past it,
  leaving the original copy unreachable; both forms are the same length, so
  the word count hides it. Moving `var_v1 = 1` out of the arms stopped two
  such duplications of the join's `move v0,v1` and matched the target's two
  `nop`s. This is the cheapest known handle on that class: a statement move
  that provably changes nothing else. Evidence: `func_8005ABA8`, five words.

- **The frame is an equation with exactly two unknowns, and the reserved
  temporary area is one of them.** For a given function
  `frame = fixed + S + L`, where `fixed` is the outgoing-argument area plus
  the saved registers, `L` is cfe's declared-local block laid top-down from
  the frame top in declaration order, and `S` is the reserved
  register-temporary area between them. The declared block's *bottom* is
  therefore `frame - L`, which is where the last declared local sits, and
  every earlier declaration follows upward from it. Because `spilltemps`
  reserves a slot per temporary whether or not it ever touches memory, `S` is
  almost entirely invisible in the disassembly -- a function can reserve
  twelve slots and write one. So a frame that is wrong by a constant is not
  necessarily a declaration-census question: solve for `L` from the last
  local's offset and for `S` from the frame, and the two numbers say whether
  to add declarations or to remove temporaries. Evidence: `func_80030610` in
  `src/main/sched.c`, where the target is `L = 40, S = 40` and the candidate
  `L = 24, S = 48`; the previous reading had concluded the target needed four
  *more* spilled values when it reserves two fewer temporaries.
- **A loop is not a hand-unrolled chain, and the difference is the initial
  induction pointer.** Where a target walks a short fixed-length run of fields,
  a manually unrolled read chain and a real counted loop over the same cursor
  do not compile alike. With a hand-written chain uopt folds the base into the
  first load and materialises the pointer only for the increment, so the first
  read never goes through it (`lh 0x3C(a1)`, then `addiu a3, a1, 0x3C`). IDO's
  own unroller materialises the initial induction pointer first, reads through
  it, and forms the next cursor from it (`addiu a3, a1, 0x3C`; `lh 0(a3)`;
  `addiu a2, a3, 2`), which is the shape the target has, including the folded
  second increment and the plain `addiu` chain after it. `volatile` on the
  cursor is a second, independent requirement: it is what keeps the unrolled
  loads from folding their bases, and without it the same loop costs 29 words
  in the scan tail. `for` and bottom-tested `do` are byte-identical here, as
  are the pointer and `p[0]` read spellings and every way of writing the base.
  Measured on `overlay97InitScale`, whose single differing word survived a
  192-cell lattice of separate-carrier spellings, four qualifier sets, four
  cursor bases and both assignment orders, and closed the moment the chain
  became a loop. Limits: the lever is about the *initial* pointer, so it does
  nothing where the target's first read is itself folded.

- **A copy pair whose two colours are swapped is a numbering problem, and the
  numbering lever is deleting the name.** Where the target loads a global into
  one register and copies it into another, and the candidate has the pair the
  other way round, spelling both values as named locals cannot fix it: both
  take pool colours in first-definition order, so whichever name the load
  defines takes the lower colour and the two spellings only ever swap the pair.
  On `overlay40FadeRecords` that is exactly what the two natural forms do
  (3 words one way, 16 the other, with the disputed rows byte-identical between
  them), and it survived a 440-cell declaration-order x origin cross product
  that was perfectly flat. Deleting the second local and spelling its single
  use as the global read *again* makes the loaded value a CSE temporary, which
  uopt numbers after every named local: it takes the higher colour and the
  surviving local, now defined by the copy, takes the lower one. Note the
  distinction from the recorded failure mode: keeping the local declared *and*
  assigned from the global leaves two named locals and forces a second `%hi`
  materialisation (+1 word). The name has to go, not just its origin.

- **uopt's interference is basic-block coarse, so removing a block can hand a
  web a physical-register forbid for free.** A web whose definition lands in
  the *entry* block is treated as live from function entry, and therefore
  interferes with the incoming parameters' physical registers. On
  `func_80009AA8` the three-word residual was one caller-saved colour that no
  amount of web-count or priority work could move; collapsing
  `x = 0; if (cond) { x = 1; }` into `x = cond` removes two basic blocks, which
  puts the list pointer's and the selected entry's definitions in block 0,
  which makes them interfere with the incoming `$a0` the object pointer arrives
  in -- and the residual web takes `a1`. The trace shows it plainly: the web's
  own interference list still names only four assigned neighbours while its
  decision line forbids five colours, and its recorded basic block goes 2 -> 0.
  This is a block-count lever, not a web-count lever. Its limit is the reason
  it does not transfer: a function whose float parameters arrive in `$f12`/
  `$f14` has no incoming integer argument web to pick up, and a definition that
  follows a call can never be in block 0 because the call ends the block.

- **A caller-saved colour is forbidden for a web only by a pool web of that
  colour or by a physical web it overlaps, and a call-crossing web can never
  supply one.** A web that crosses a call has only callee-saved colours in its
  `available` set -- forcing it to an argument register is silently ignored --
  so any "extra web at `a0`" has to live entirely between two calls. Combined
  with the ordering rule above, that reduces an argument-register residual to a
  single concrete question: is there a zero-cost web, defined and dead between
  the same pair of calls, that can be given the colour first?

- **The way to build that web is a copy back into an existing carrier.** An
  invisible coloured web -- one that takes a colour and emits no instruction --
  is rare and worth recognising: a census of all 65 procedures in
  `src/main/objects.c` found exactly four, and two of them are a coalesced copy
  through a local (`temp = sp58 + 0x100; sp58 = temp;`) and a pass-through
  parameter. Writing the accumulated value *back into the carrier it came
  from*, and reading the next value out of that carrier, is what makes one:
  `temp_v0 += var_s1; var_s2 = (u8 *)temp_v0;` in place of
  `var_s2 = (u8 *)temp_v0 + var_s1;` closed both `func_80004454` and
  `func_8000471C` at no instruction cost. The distinction is sharp and cost the
  search a long detour: copies into a *fresh* carrier all coalesce to nothing
  and reserve no colour.

- **`(relational) == 0` burns a ugen ring temp and emits nothing, which makes
  the ring phase a source-level dial.** ugen materialises such a test as a
  `seq`/`beq` pair and `as1` fuses it back into one branch, so the temporary is
  consumed but never emitted. `!(x)`, `(x) != 0` and `(x) != 0U` all fold back
  to a bare branch and burn nothing; `== 1`, `!= 1` and `^ 1` emit a real
  instruction and cost two words. Three settings, usable at any branch site,
  in either direction. On `func_8005A948` the target burned its ring temp at
  the loop guard rather than at the inner compare, and moving the normalisation
  from one to the other resynced every later temp: the guard edit alone is 23
  differing words, dropping the inner `!= 0U` alone is 26, and the pair is
  exact. Limit: this moves the ugen ring only. It never moves a uopt pool
  colour -- measured on `func_80004454`, where the same dial shifts the ring by
  15 words and leaves the caller-saved residual untouched.

  A fourth setting, on the same dial and at zero width: a single-bit test
  spelled `((x << N) & 0x80000000U) == 0` burns one ring temp and emits
  nothing, where `!(x & bit)` burns none. Both compile to the same two words
  -- ugen already lowers `!(x & bit)` to a shift-to-the-sign-bit plus
  `bltzl`/`bgezl`, so the shift is written either way -- but the explicit
  `== 0` against the masked high bit consumes a temporary the implicit form
  does not. Use it wherever a bit test sits upstream of the ring phase you
  need to advance. Measured on `func_overlay_079_F0000134_18CD0D4`, where the
  target's `sll t3` against the candidate's `sll t2` at +0x58 is that one
  temporary and this spelling is the only one of thirteen tried that supplies
  it silently: the forms that put a real `sltiu` into the ring (`(y < k) == 0`,
  `(y != k) == 0`, `(y ^ k) == 0`) advance it too but cost forty-plus words,
  and `!(...)`, `(...) != 0`, `>= k`, `> k-1` and the `>> n & mask` rewrite are
  all flat. The advance is only worth taking with the matching give-back
  downstream; on its own it measured 328 against a 288 base.

- **`globalcolor` picks the lowest free colour among equals, so an `a0`-versus-
  `a1` residual is an interference problem and never a priority one.** Read
  directly from the instrumented `uopt` on three `objects.c` functions: every
  caller-saved colour carries `cost=0.000000` for a web that crosses no call,
  and uopt scans colours in ascending order (1=v0, 2=v1, 3=a0, 4=a1, 5=a2,
  6=a3, 7..12=t0..t5, 14..22=s0..s8) taking the first minimum. Candidates are
  visited by `save` descending with ties broken by ascending web number. The
  consequence is a search-space theorem: if the residual web already sits at
  the bottom of the priority order, nothing that changes its *priority* can
  move its colour, because being decided earlier only makes the lower colour
  more free. The only reachable fix is one additional web that (a) interferes
  with it, (b) is decided before it, (c) crosses no call so its caller-saved
  cost is zero, and (d) already has the colours below the wanted one forbidden.
  Such a web need not be visible: `func_80004454` has a web coloured `v1` whose
  register never appears anywhere in the emitted function. Use this to retire
  the whole declaration-order, statement-order, operand-order and dead-store
  space in one reading instead of grinding it -- on `func_80004454` those
  spaces are now measured flat over about 1,900 candidates.

- **When a pool colour is wrong, move the *other* web, not the carrier.** A
  residual that is one pool colour is a two-body problem: the carrier and
  whichever web already holds the colour you want. uopt colours pool webs by
  descending `references / bucket(references + spanning statements)`, so
  whichever of the two is coloured first takes the lower register and the other
  takes the next one. Every spelling of the *carrier* leaves that order intact,
  which is why carrier type, position, web-split and dead-store lattices read
  as flat; ending the *other* web's live range before the carrier's definition
  closes it in one edit. The cheapest way to end a local's range early is to
  read the field back where the local was about to be used again -- uopt
  forwards the store on the line above to that load, so the read costs no
  instruction. Evidence: `func_80049000` in `src/main/fx.c`, five words for
  four passes, where `mode`'s last use was the vertex multiply's
  `(mode & 0xFF)` and ugen schedules the height `l.s`/`trunc.w.s`/`mfc1` ahead
  of that multiply; spelling the multiply's base `ripple->mode` -- the field
  the line above has just written -- ends `mode` at the store, and `height`
  takes `v1`. 75 order x base cells, exactly two exact. Read `cc -S` to see
  which web actually overlaps: the interference is against ugen's schedule, not
  against source order.

- **A block's colours can be held open by giving the *previous* block one more
  statement, and the statement carrier is the physical line.** A CSE that is
  one or more colours too low is not always a missing reservation: it can be a
  neighbouring pool web that died too early. Moving the *next* block's setup
  statements onto the previous block's statement line emits them while that
  block's web is still live, removes its colour from the CSE's admissible set,
  and moves the CSE up the ladder without adding a web, a temporary or a frame
  slot. Evidence: `func_overlay_029_F00010C4_187E374`, four words and a
  4,000-form randomised sweep that reached `v0`, `v1` and `t5..t9` and never
  `a0`-`a3`; placing the second block's two `angles[...]` assignments at the
  end of the first block's initialiser macro keeps `record` (the first block's
  `v1` web) live across the shared `verticalAngle + 0x2000` and the carrier
  moves to `a2`. Position inside the initialiser is load-bearing -- only the
  last of twelve insertion points is exact, the rest cost 13 to 45 words.

- **A nested macro expansion is not the same line as a flat one.** Wrapping an
  existing multi-line macro in another (`do { INNER(x); next_; } while (0)`)
  resynchronises acpp's line counter, so the trailing statements get their own
  `.loc` and a different schedule; spelling the body out inside the new macro
  keeps them on the invocation's line. Writing the same statements after the
  macro invocation on one physical source line behaves like the nested form,
  not like the flat one. Measured on `func_overlay_029_F00010C4_187E374`: flat
  macro exact, nested macro and same-line-after both 17 words. When a line-group
  result matters, verify it against the object rather than assuming cpp keeps
  your grouping.

- **`globalcolor` runs phase one for a procedure that contains a call and
  phase two for a leaf, and never both.** Read from the instrumented `uopt`
  index capture (`CDX_LOG=1` with a nonnumeric `CDX_PROC`) over three whole
  translation units -- 61 procedures in `overlay_001_tail.c`,
  `overlay_008.c` and `overlay52TailB.c`. Every procedure whose body issues at
  least one call emits `p1dec` records and zero `p2dec`; every leaf emits
  `p2dec` and zero `p1dec`; a procedure with no allocator decisions emits
  neither. 59 of the 61 classify and the two that do not emit no allocator
  decision at all; there are no counterexamples.
  The consequence is a search-space theorem for the whole queue: the phase-two
  web-numbering axis -- first-definition order, coloured ascending, so moving a
  defining statement moves a colour -- is **dead for any function containing a
  call**, which is every whale left in the ranking. Run the index capture and
  read the phase before spending a lattice on declaration, definition or
  statement position.

- **The class-2 (floating) colour table is c24=`f0`, c25=`f2`, c26=`f12`,
  c27=`f14`, c28=`f16`, c29=`f18`; c30 and above are callee-saved and pay
  prologue words.** Established by force-and-diff on
  `func_overlay_008_F00034A0_18611F8` (uopt procedure 14): its web 187 sits at
  c24 and, forced through c25..c29 one at a time, its register moved to `f2`,
  `f12`, `f14`, `f16`, `f18` in that order; web 29, which sits at c26, forced
  to c27 swapped `f12` with `f14` against its neighbour, confirming both
  entries from a second web; c30 grew the function by a word. The pool is six
  wide (`available0=0x000000fc`, bit index 31 - colour). This extends the
  workbench's integer `COLOR_REGISTERS` decode, which stops at c23, and it is
  what makes a floating residual readable: a difference among these six is an
  allocator decision, and a difference outside them is not.

- **`f4`, `f6`, `f8`, `f10` and `t3`..`t9` are never globalcolor colours in
  these procedures; they are ugen expression temporaries.** No force can name
  one of them directly, so a residual spelled there is not a colour the
  allocator picked. Measured on the aligned naming residual of three whales:
  622 of 739 differing pairs in `func_overlay_052_F000063C_189ACAC` name a
  register `globalcolor` never assigned in that procedure, 209 of 557 in
  `func_overlay_001_F000438C_185076C`, 91 of 209 in
  `func_overlay_008_F00034A0_18611F8`; the purely allocator-named share is 48,
  36 and 13. **That census is not a bound on what the allocator can reach**,
  and reading it as one is a mistake this lane made and then measured its way
  out of. The ring's *phase* is set by how many pool colours are consumed
  upstream of it, so one colour change rotates the ring downstream: forcing a
  single class-2 web on `func_overlay_008...` rotated the whole
  `f4`/`f6`/`f8`/`f10` ring in one compile, and the greedy ceiling below
  closed 127 naming rows on `func_overlay_001...` where the direct census
  names 123 and 156 on `func_overlay_052...` where it names 67. Use the census
  to say what *kind* of decision a row is, and the ceiling to say how much is
  reachable.

- **Size the allocator's share of a residual with a greedy force ceiling
  before opening any allocator lattice.** Sweep every web against every colour
  and the split path, keep the best, repeat with that force as a prefix. The
  instrumented compiler runs whole translation units at about 35 a second
  under `xargs -P8`, so a 736-cell round on a 21-procedure unit costs 21
  seconds:

  ```sh
  CDX_LOG=1 CDX_PROC=<n> CDX_FORCE=p1:w<web>=<c<colour>|s> CDX_OUT=/dev/null \
    IDO_DIR=~/Desktop/dev/ido-instrumented \
    .venv/bin/python tools/ido-phases.py <the configured flags>
  ```

  Three whales came out within two points of each other, which is the useful
  part. `func_overlay_008_F00034A0_18611F8` moved from 592 byte-exact / 209
  naming / 101 different to 642 / 167 / 94 over five rounds and the sixth
  found nothing: 50 recovered words of the 310 wrong, 16%.
  `func_overlay_052_F000063C_189ACAC` reached 156 of 1031 in three rounds,
  15%, and `func_overlay_001_F000438C_185076C` 136 of 822 in four, 17%, each
  with the next round flat. **Budget about a sixth of a whale's residual to
  the allocator and spend the rest elsewhere.** Two cautions the sweep also
  settled. A single force closed the -4 size deficit on two of the three
  functions, so a candidate one instruction short is not automatically missing
  a source operation -- it can be one web the target spills and the candidate
  keeps. And a force that reports `forced=-2` in its own `p1color` record
  never applied, so its byte-identical object proves nothing; check the
  record, not the object.
- **uopt's `<`-to-`!=` loop-exit rewrite (EQ_INEQ) runs before copy
  propagation, so a counter initialised from a local copy of a constant keeps
  its `slti`.** The rewrite needs a constant initial value at the moment it
  runs; `i = zero` where `zero = 0` was assigned earlier is not one, so the
  test survives as `slti` against the literal bound, and the later propagation
  still emits `move i, zero`. A literal `i = 0` is rewritten for every loop
  spelling (nine test forms, all statement orders). A register-resident bound
  (`i < n`) also blocks the rewrite but is *not* folded afterwards: the bound
  stays a hoisted register and the compare a `slt`. Evidence: `func_8003A754`,
  where this was the structural half of a twelve-word residual; the `uoptlist`
  phase listing names the pass (`EQ_INEQ at BB:`) ahead of `COPY PROPAGATION`.
- **A call result kept live to the exit holds `v0` through the whole body,
  and uopt deletes an empty conditional after liveness.** With every web
  otherwise identical, `func_8003A754`'s target had no web on `v0` at all and
  every colour shifted by one from the candidate. Keeping the returned pointer
  live to the exit with a trailing `if (walked != base) { }` reproduced the
  target's seven colours exactly and emitted no instruction: the branch is
  folded away after interference is computed. `return base` does not do it (it
  creates a copy web coloured last), and neither does a non-void return type
  without a return statement. The trailing compare's extra site also decided a
  save tie between the byte-offset counter and the walked pointer, so the
  offset could be defined first (as1 emits its move first) and still be
  coloured second. Diagnostic-only until a natural source form is found; the
  committed function says so at the point of use.

### Assembler scheduling and phase replay

- The `cc -S` listing is a faithful, editable stand-in for what `as1` receives.
  `ugen` accepts a `-l <file>` argument that writes its output as text, which
  the driver uses for `-S`; re-assembling that text with `acpp` + `as0` + `as1`
  under the *compiler-path* `as1` flags reproduces the compiler's own object
  byte-for-byte. The text path invoked through the driver does **not**, because
  the driver adds `-pic0 -noglobal` to `as1` for a `.s` input; drop those two
  and the round trip is exact. This turns the last phase into a directly
  searchable space: edit the listing, re-assemble, score, and you learn what
  `as1` input the target requires before spending any time guessing C.
  Measured exact on two overlay units (a 301-word and a 262-word candidate).
  Limits: only the final phase is replayed, so a residual owned by `uopt` or
  `ugen` is unaffected, and the listing must be re-derived after every source
  edit.

- **A promotion must preserve the physical line count, or it moves the next
  function.** Removing a `#ifdef NON_MATCHING` guard deletes four lines and
  rewriting the plateau comment above it usually deletes more; every function
  *below* then compiles at a different source line, and IDO's schedule is
  sensitive to that. Measured: promoting `func_8005A948` with a comment eight
  lines shorter left that function exact and moved `func_8005AAC0` by twelve
  pure register renames, first visible at ROM 0x5B700. The ROM checksum gate
  caught it, but the cheap check is to compile the whole translation unit
  before and after and compare every symbol's `.text`, expecting exactly one to
  differ. Absorb the guard's lines into the comment that replaces it.

- **`globalcolor`'s two sweeps use two different priority orders, and only one
  of them is `save`.** The phase-one (callee-saved) sweep takes webs in
  strictly descending `save`; the phase-two (caller-saved) sweep takes them in
  **ascending web number**, and each web then gets the lowest colour no
  already-coloured interferer holds. Measured on two functions in the same
  session: `overlay1ResolvePathPoint`'s nineteen p1 decisions come in
  descending `save` (30.5, 30.0, 20.0, 8.5, 5.0, 5.0, 3.0, ... 0.5), while
  every one of `overlay20RemoveEntry`'s twelve p2 records reproduces its logged
  `forbidden0` under ascending web number and under no other order -- descending
  `save` predicts the first web's forbidden set as three colours where the log
  shows one. In p2, `save` is not the priority at all; it is the gate that
  decides `color` against `no-color`. The consequence for matching is direct:
  a caller-saved residual is a *numbering* problem, not a weight problem, so
  the lever is whatever moves the web's position in uopt's symbol order, and
  reasoning about it from the descending-`save` law predicts the wrong web
  first every time. Decode the masks as bit `31 - c`, with c1..c6 = v0, v1, a0,
  a1, a2, a3.

- **uopt forwards a call's own return register into every use in the block
  that contains the call, so which basic block a statement lives in decides
  whether it reads the raw result or the saved copy.** This is a decision
  variable that no amount of respelling a statement can reach, and it is
  invisible in a spelling lattice because every cell of such a lattice holds
  the statement's block fixed. `overlay1ResolvePathPoint` sat one word from a
  match for three lanes on exactly this: a default cursor assignment written
  before an `if` shared the call's block and took the return register, where
  the target reads the callee-saved copy. Deleting it and the two per-arm
  assignments, and writing one assignment after the if/else so it lands in the
  join block, closed the function with no other change. Before spending a lane
  on spellings of a base-register residual near a call, ask which block the
  statement is in and whether a semantically identical placement puts it in
  another one.

- **Reading `globalcolor` needs the procedure *ordinal*, and the ordinal is the
  function's index in `.text` address order.** The instrumented `uopt`
  (`~/Desktop/dev/ido-instrumented/cc`, `CDX_LOG=1`) refuses a symbol name:
  it sees ordinals, not linker names. Sorting the compiled object's `FUNC`
  symbols by address and numbering them from zero reproduces the ordinals
  exactly -- on `src/main/objects.c` that is 65 procedures against 65
  `globalcolor` blocks, and the mapping was confirmed by forcing a web in the
  predicted ordinal and watching the predicted function change. Naive counting
  of C function definitions in the source does **not** work: multi-line
  signatures are missed and `#pragma GLOBAL_ASM` bodies contribute no
  procedure. With the ordinal in hand, `CDX_DETAIL_WEB=<n>` prints the web's
  type, its cfe stack offset and its full interference list with each
  neighbour's assigned colour, which is what turns a register residual from a
  guessing game into a stated requirement.

- `ugen` emits caller-save spill stores around a call in strictly ascending
  register order. Census of every call site in the tree whose store pair is
  reloaded unchanged immediately after the call: 73 of 73 ascending, with
  register order winning over home-offset order in the 38 sites where the two
  disagree. A target that shows the opposite order therefore cannot be reached
  by any declaration order, statement order, loop form, or physical-line
  grouping; the difference has to be created downstream in `as1` or the pair is
  not a spill pair at all. Use this to retire a whole search space in one
  measurement instead of grinding source forms.

- `as1` reorders an adjacent pair of independent stack stores that ends a
  basic block before a call, putting the *first* of the pair in the delay slot,
  when a may-alias load sits in front of them in the same line region. Replace
  that load with one whose stack displacement is provably distinct and the pair
  keeps its order, so the trigger is the assembler's memory disambiguation, not
  the registers or the displacements (both were swept without effect). Of the
  39 real store-pair sites in the tree, 30 keep and 9 reverse. Consequence for
  matching: an ascending `ugen` pair reaches the object reversed, and a target
  showing it un-reversed needs a barrier the source cannot always supply.

- A `.loc` naming a **greater** line than the current one is a backward-motion
  barrier for the `as1` scheduler; a repeated `.loc` on the same line, a
  smaller line, or a `.livereg` is not. Proved by inserting each form between
  two otherwise identical stores: only the increasing `.loc` stopped the swap.
  This is the mechanism under the already-recorded observation that physical
  source line grouping changes the schedule -- joining statements onto one line
  removes a barrier, splitting them adds one. Its limit is important: `ugen`
  emits at most one `.loc` per statement, and the spill stores for a call all
  belong to the call's statement, so no source spelling can put a barrier
  *inside* a spill group. Merging lines also does not always help, because
  `ugen` still emits a same-line `.loc` per statement, which is not a barrier.

- Debug context alone changes the schedule. A listing with no `.file` at all
  schedules differently from the same instructions with one, independently of
  any `.loc`. Since every C compile emits a `.file`, this is a diagnosis aid,
  not a lever: when a minimal hand-written reproduction of a residual behaves
  differently from the real unit, check for the debug directives before
  concluding the surrounding code is responsible.

- `$at` in the emitted code is a signature, not an allocation. `ugen` only
  writes `$at` inside its own `.set noat` sequences (64-bit arithmetic); every
  other appearance comes from an `as1` macro expansion. In particular a
  multiply by a constant is expanded by `as1` using `$at` as the scratch, and
  two multiplies by negative constants in the same block share a single
  negation into `$at`. So a target that negates into `$at` and shifts out of it
  proves the source wrote a multiplication, not the equivalent shift of a
  negated operand -- the shift spelling makes `ugen` strength-reduce into a
  pool register and `$at` can never appear. Evidence: `overlay62Update`, where
  this converted a mixed structural residual into a pure register rotation.

- **`ugen` emits a pre-loop statement's code before the loop preheader's
  hoisted invariants, always.** Source order, physical line grouping and
  spelling cannot reorder the two: on `overlay59Advance`, all 64 line
  groupings of the six entry statements are flat, including every statement
  merged onto the loop-header line (verified in the phase output to collapse to
  one `.loc` region). Moving the statement inside the loop does put it after
  the invariants, but `uopt` then declines to hoist a global load out of a loop
  that contains calls, and `const` on the global does not change that in IDO
  5.3. So when a target materialises a hoisted constant *before* a global load
  into a saved register, the source is not simply a reordering of the entry
  block, and sweeping statement order there is wasted effort.

- **Prologue save order is nearly free; do not search it.** `as1` re-schedules
  the save block from the order of the value definitions, so the order it
  arrives in from `ugen` barely reaches the object. Measured on
  `overlay59Advance`: all six permutations of three `s.d` saves, each with the
  FP group before and after the ten integer saves, move a 19-word residual to
  18 at best. A prologue residual is an ordering question about the *defs*, not
  the saves.

- **`as1` fills an annulled (branch-likely) delay slot from the first
  instruction of the branch-target block, and leaves the original in place.**
  So whatever the join block begins with is emitted twice, once in the slot and
  once on the fall-through. A target that appears to read the same field twice
  near a likely branch is usually not a source-level repeated read at all; it
  is one load that owns the join block. The lever is therefore to change *what
  the join block starts with*, not how the test is written. Evidence:
  `overlay75UpdateMovingObject`, where reading this as a repeated field access
  sent an earlier pass at the test site instead of at the initialisers.

- **When `as1` declines that duplication, and what can stop it.** Measured
  across the ten conditional branches of `func_8005ABA8` plus a directed sweep
  of its phase input. It declines in exactly three situations: the target
  block's first *scheduled* instruction is itself a branch; that instruction
  has been hoisted out from under its own label by an earlier `as1`
  transformation, leaving the label on an empty block (retargeting the branch
  to the next label restores the conversion, which is how the condition was
  identified); or a **location-counter directive stands at the block head**.
  The suppressing set is exactly `.align n`, `.space 0` and `.text`; `.loc`
  (any line, inserted, deleted or moved), `.livereg`, `.noalias`/`.alias`,
  `.mask`, `.frame`, `.file`, `.option`, `.verstamp`, an added label, a padding
  instruction and a redundant jump are all inert at that position. Two
  consequences. First, **physical line grouping cannot flip a delay-slot fill**
  -- grouping moves only `.loc` lines. Second, since `ugen` emits `.align` and
  `.text` only at function starts, a target that keeps its own delay slot where
  the candidate duplicates a block head is not reachable by adding a barrier;
  the difference has to be somewhere else in the phase input. Liveness of the
  duplicated destination on the fall-through path is not a factor, and neither
  is the instruction's opcode class: integer branches duplicate float
  arithmetic freely.

- **`as1` has no cross-function state.** Deleting any other function from the
  listing, at any position, leaves a given function's scheduling and delay-slot
  decisions unchanged. A residual owned by the last phase is therefore a
  question about that function's own phase input, and the search space never
  includes its neighbours.

- **A cycle in a window's register substitutions does not prove a FIFO
  phase.** A window can span several basic blocks and unrelated live ranges;
  choosing each register's most frequent replacement can close a graph cycle
  even when none of those registers is drawn from the temporary free list.
  First identify actual pool assignments and the traced temporary-register
  set, then locate the first divergent draw relative to the first mismatch.
  A controlled entry experiment moved the FIFO at zero instruction cost while
  repairing none of the existing naming rows; its first mismatch preceded the
  first temporary draw. Use a cycle to nominate a hypothesis, not to infer a
  shared cause or a reachable-word bound. A pool-colour change can still
  affect downstream FIFO membership, so that interaction must be measured
  separately. See the [entry-cycle control](whale-entry-phase.md).

- **The `ugen` temporary ring is a FIFO free list.** A temporary pops the head
  of the queue and is pushed back on the tail when it dies, so a short-lived
  temporary that emits **no instruction at all** -- for example the boolean
  temporary a `(a == b) != 0U` normalization creates, which `as1` folds back
  into the branch -- still consumes a pop and renumbers every temporary after
  it. That makes such a normalization a one-notch dial on the whole downstream
  ring, and it is why a candidate can be exact everywhere except for one pair
  of temporaries whose creation order differs. Diagnose it by reading the pops
  off both sides in order; if every pop after the disputed pair agrees, the
  residual is a creation-order question at that pair, not a phase or an
  allocator question. Evidence: `func_8005A948`, where patching the two names
  in the phase input takes a three-word residual to zero.

- **`as1` picks a call's delay-slot filler by memory disambiguation against
  `$sp`, not by emission index or source line.** Where the register holding a
  call's first argument is one `as1` cannot prove disjoint from the stack, it
  sinks the *earlier* of two adjacent caller-save spill stores into the delay
  slot and leaves the later one in front of the call, inverting `ugen`'s
  emission order. Three byte-inert facts restore that order, each measured on
  `overlay11UpdateMenu` (1,204 bytes, 301 words) where the inversion is the
  entire two-word residual and each perturbation scores exact under listing
  replay: a `.noalias <reg>,$sp` naming the argument register, anywhere from
  the loop preheader through the point between the two stores; a `.loc` naming
  a greater line between the stores; and `.set volatile` around the **first**
  store. Inert: `.noalias` naming any other register; `.noalias` placed after
  both stores; `.noalias` opened before the load and closed with `.alias`
  before the second store, so the fact has to hold *at* that store;
  `.set volatile` around the second store alone; and `.livereg` in every form
  tried -- moved ahead of the pair, deleted, and with two other masks. This is
  a different position and a different decision from the block-head duplication
  question above, where `.noalias` is measured inert; the two results do not
  conflict.

- **A `.noalias` fact is useful only for the relevant register value and
  scheduling region.** Symptom: a named-object indexing rewrite leaves a
  may-alias scheduling residual unchanged even though the listing contains
  the desired register pair. Mechanism: the register is reused for a different
  pointer later in the function, and its fact covers only that later
  lifetime. A fact after the disputed stores cannot permit an earlier load
  to cross them. A walking pointer can lack a fact even when it was initialized
  from a named array; the same physical register can acquire a fact later when
  a different static address is materialized and dereferenced.

  Lever: read every `.noalias` and matching `.alias` in a faithful `cc -S`
  listing, identify the argument load's **base**, and locate the fact relative
  to the precise scheduling decision. Replay controls must distinguish the
  right base register, a wrong register, and a fact closed before the decision.
  Measure the full text and relocation table against the configured object
  before trusting that replay. An assembly-only zero is diagnostic, never a
  C match.

  Limits: named-array indexing is not a universal syntactic test for an
  explicit directive. In a measured full TU, symbolic indexed memory operands
  emit no explicit fact, while both a decay-plus-offset address and its indexed
  equivalent already emit identical facts for a later pointer. Those two C
  forms also have identical text, relocations, frame size and `.mdebug` locals.
  Do not infer a universal temporary-cell cost or C unreachability from other
  indexed forms that happened to add temps. Read `cc -g3`'s ECOFF local table
  for each relevant candidate and distinguish declared locals from compiler
  temporaries. Debug mode can change code bytes even when the frame agrees;
  its allocation table is secondary evidence, and configured stock output
  remains the matching authority.

- **`cc -S` ignores `-o`.** The listing is written to the *current directory*
  under the source's base name. Move it into a scratch directory as the next
  command; a stray `.s` in the worktree root is exactly the kind of file the
  clean-room rules exist to keep out of a commit.

### Search fidelity and false floors

- A permuter zero is a hypothesis until the project pipeline verifies it.
  Function isolation must reproduce the TU's flags and metadata-only object
  steps, and stack-slot differences must be scored. Evidence:
  `docs/matching-triage.md` section "Permuter scratch fidelity."
- Do not call a register/scheduler residual unreachable until the correct ISA,
  faithful scratch build, bounded permuter, and workbench route have all been
  tried. Several Mickey functions first classified as hard scheduler walls
  became exact after those preconditions were repaired. Evidence:
  `docs/matching-triage.md`.
- Permuter output is a source-spelling probe, not a style authority. Preserve
  an exact but inert spelling when necessary, disclose it in source, and put
  naturalization in `docs/cleanup-queue.md`; never keep a semantic mutation
  merely because it scores better. Evidence: ADR 0007 and the cleanup queue.
- **An unpaired `%hi`/`%lo` in the assembled fallback is a permanent false
  residual, and every object-level scorer in this project reports it.** splat
  can only pair a high/low address pair it can see adjacent; when IDO hoists
  the `lui` above a branch and leaves the load after it, splat writes both
  halves as raw literals. The fallback object then carries no relocation
  there, a correct candidate carries two, and their raw immediate fields
  cannot agree however right the C is. `func_800056A4` carried such a two-word
  "register difference" through three separate work packets, including a
  forced-color diagnosis and a flat permuter sweep, and matched on the first
  attempt once the address was spelled as the symbol it actually is. The
  address in that case sat *inside* a neighbouring symbol's extent, so the
  spelling is `&D_800C9460[1]`, not a new name: resolve bss addresses through
  the map, never through the name splat happened to mint. Cheap detector:
  score with every relocation on both sides resolved against the canonical
  linked ELF's symbol addresses before comparing words, which makes the two
  spellings compare equal exactly when the linked bytes are equal. Suspect
  this whenever a fallback shows a bare `lui` of a plausible RAM address whose
  paired load is on the other side of a branch.
- **The `-O2` frame is an equation over declared locals and compiler
  temporaries, and both halves move it.** Measured on `func_8000784C` across a
  24-point grid of declared-scalar counts and aggregate sizes:
  `frame = roundup8(fixed + 4*scalars + aggregate)` and
  `aggregate_base = frame - 4*scalars - aggregate`, where `fixed` is the
  outgoing-argument area plus the saved registers plus the compiler-temporary
  block. Every declared scalar reserves four bytes above the aggregates
  whether or not it is ever touched through the stack, so an unused local is
  as load-bearing as a used one. The corollary that costs time if missed:
  adding a local moves an aggregate *up*, not down. To move one down while
  holding the frame, a declared local must also *displace* a compiler
  temporary -- naming a value the compiler was going to materialise anyway --
  because the two blocks trade against each other. `fixed` itself responds to
  the widest outgoing call: dropping a five-argument call to four moves the
  saved registers and everything above them down eight bytes.
  **The displaceable temporaries are loop-invariant common subexpressions in a
  loop's controlling condition.** `func_8000784C` matched once the wrap loop's
  twice-spelled halfword read became a named local: the plain
  `while (a->c >= f(a)) { a->c -= f(a); }` spends *two* compiler temporaries on
  that subexpression, and naming the value pays both back while costing one
  scalar -- exactly the ninth-scalar-with-a-freed-temporary trade the equation
  wants, where sixteen probes that added a scalar anywhere else all moved the
  aggregate the wrong way. Naming it naively also deletes the in-loop store and
  reload, so keep the field read *in the condition*
  (`while (a->c >= (period = f(a)))`) and use the name only in the body: the
  emitted words are then identical and only the frame's split changes. Removing
  such a `while` outright is the cheap detector -- if the aggregate's home drops
  by 8 with the loop gone, its condition owns two temporaries.

- **The commutative-weight rule reaches a float multiply, and an explicit
  `(f32)` cast is the lever there.** Upstream L92 and field-guide lever 54 are
  both written against integer/pointer arithmetic -- cast an address base to
  `s32` and ugen sums it base-first. The same rule governs `mul.s`. On
  `func_8005716C` the x-axis product's two float operands are byte-identical
  written either way (`normalX * (...)` and `(...) * normalX` emit the same
  word, which is why an earlier pass filed that word as not source-reachable),
  and an explicit `(f32)` cast on `normalX` -- a no-op on an `f32` local --
  changes that operand's weight and moves it to the left. The cast on the
  *right* operand is inert, and so is a unary `+`; only the left one moves the
  word. Worth one word there, and it was the second of the two edits that took
  the function to byte-identical. File it upstream as a receipt on L92 rather
  than as a new law.
- **The frame's cell census is readable directly out of uopt, not inferred
  from frame sizes.** `cc -Wo,-zdbug:2` writes a `uoptlist` whose global-
  colouring section prints one `isvar M <class> <offset>` row per declared
  local and one `isvar P` row per parameter (never commit it -- it is
  ROM-adjacent build output and gitignored nowhere). Read against the emitted
  frame it gives the cell law for a function in one compile: with N cells the
  frame is `align8(4N)` and the last cell homes at `align8(4N) - 4N`, so a
  target home fixes N's parity. The trap it exposes is that dropping a
  declaration and dropping a *cell* are not the same move: on `func_8005716C`,
  removing the `normalX` local and re-reading `normal->x` keeps the census at
  ten because the re-read's own web takes the freed slot. The move that works
  is to trade a declaration for a compiler temporary the compiler was going to
  create anyway -- there, spelling a doubling as `-x * 2.0f`, which uopt
  rewrites into the sum it was already emitting.

- **as1 inverts a pair of stores that one memory node releases together, and
  that is a graph shape rather than a tie-break.** `cc -Wa,-R` prints the
  block's dependence graph and every selection. When an argument load that the
  assembler cannot prove disjoint from the stack sits above a caller-save spill
  group, *both* stores hang off the load; scheduling the load releases them in
  the same step, they enter the ready list in **reverse emission order**, and
  the later-emitted store is picked first. Give the load a disambiguation fact
  and the stores have no predecessor at all, start in the initial ready list in
  emission order, and the order survives. So a reversed spill pair around a call
  is a *readiness* question, and no amount of statement placement, line grouping
  or liveness annotation touches it -- exactly the failure mode
  `overlay11UpdateMenu` spent three passes on. Read the trace before theorising
  about the tie-break chain; the chain only decides among nodes that are already
  ready.
- **ugen emits a call's caller-save spill stores in ascending physical register
  number, with no exceptions worth planning around.** Measured across 391
  compiled translation units: **136 of 136** clean spill groups -- a run of
  stack stores immediately before a call whose offsets are reloaded immediately
  after -- are in ascending register order. The two apparent counterexamples are
  mis-grouped, each a store whose value is reloaded into a *different* register.
  The practical consequence is that when a target's spill order disagrees with
  the candidate's, and the register assignment already matches, the emission
  order is not the lever: something downstream reordered it, and the fix is at
  the assembler's input, not at ugen's.
- **uopt appends a strength-reduced induction pointer's preheader
  initialisation after every user preheader statement.** So a loop whose cursor
  is a compiler-created induction pointer materialises its base *after* the
  counter's initialiser, while a loop whose cursor is the user's own walking
  pointer materialises it wherever the user's assignment sits. Measured on
  `overlay11UpdateMenu` across five loop shapes -- statement-order swap, `for`,
  `while`, increment at the top of the body, increment inside the exit test --
  and three placements of the initialiser: the induction initialisation is last
  in all eight. This matters because the array-index spelling that produces a
  memory-disambiguation fact is exactly the spelling that creates the induction
  pointer, so an indexed rewrite is not free at the preheader. Check it.

  **The "mutually exclusive" corollary that once followed this is retracted.**
  Emission order is not the object's order: as1 reschedules that block -- the
  `lui` moves above the counter's `li` in *both* forms -- and among ready nodes
  it breaks the tie on physical source line before list position (L59). So the
  induction initialiser can still be emitted last and reach the object first.
  On `overlay11UpdateMenu` the counter's `li` and the pointer's `lui`/`addiu`
  are one `aftercycles`/`latency` tie apart; with the counter on its own
  statement they carry consecutive line numbers and the `li` wins, and with the
  counter initialised in the `for` header all three carry the loop statement's
  line, the tie falls through to list position, and the just-released `addiu`
  wins. That is the target, and it closed the function's last two words.
  Moving the initialisation into the loop header is the structural form of
  L59's "make the lines equal"; it needs no whitespace folding and reads as
  ordinary C.
- **The pooled induction pointer takes pool cell one, not cell zero, and the
  frame equation makes the whole thing calculable.** For a function with a fixed
  block of outgoing arguments plus the return save, the frame is
  `align8(fixed + declared block + 4 * pooled temporaries)` and the pool is laid
  immediately below the declared block. Verified on five independent censuses of
  `overlay11UpdateMenu`, where the strength-reduced pointer landed on the second
  pool cell every time. Because the declared block is a census the source
  controls, a home that is wrong by a constant is solvable rather than
  searchable: pick the census that puts cell one where the target's home is. On
  that function, trimming the block from 44 to 28 bytes -- inlining one local
  into its single field read, folding two more onto an already-dead counter, and
  inlining a one-use temporary -- reproduced all four of the target's live stack
  homes and its frame at the same instruction count. The corollary is the useful
  half: a pooled temporary can never take a *declared* local's home, so if a
  target's live home sits inside the declared block, the value there is a
  declared local and the question is why the front end propagated it away, not
  where the pool starts.
- **cfe's copy propagation of a pointer-plus-constant is defeated by a variable
  array index, and only by an index it cannot fold.** On
  `func_overlay_022_F0000000_1878108`, every constant spelling of the same
  address -- byte-offset cast, integer casts, address-of on a typed field macro,
  a literal subscript on a cast element type, a pointer increment -- is
  byte-identical and leaves the value in a pooled temporary. Indexing by a
  *variable* moves it into the declared local's own home, which is where the
  target keeps it. The cost is the index reload and a multiply by the element
  size, and hoisting the index's assignment so cfe can fold it returns the
  original listing exactly, so the barrier lives in cfe's constant folding
  rather than in the subscript syntax. Useful as a diagnostic for "is this
  residual a slot or a propagation", cheap to run, and not yet free.

- **A live range is formed per symbol, so a second use of an existing local
  elsewhere in the function imports that region's interference into a web at
  zero width.** `uopt` builds a live range from the union of every chain that
  reads or writes one symbol, and `f_intfering` intersects the *block sets* of
  two ranges. So carrying a loop value in a local that already has a live
  range in a later loop -- rather than in a fresh local, which has no second
  range -- gives the loop's web the later loop's interferers and moves its
  colour. On `levelInit` (2026-09-11) three closures had proved that no new
  web could be added (a register-pressure cliff at s6) and none reordered;
  carrying the resource id in `shouldPlay`, whose other live range sits beside
  the v0/v1 temps of the first tune loop, took the loop value from v0 to the
  target's a2 and the function 22 -> 6 at delta 0, frame and relocations
  unchanged. Carrier identity is the whole lever: of nine declared locals only
  one reaches it, and a parameter is precoloured only on its entry chain. The
  bound, measured the same day: the value has to *survive copy propagation*
  (four distinct reads did; a single-use mask carried the same way was
  substituted back into its address expression and formed no web, so L102
  holds for a shared symbol as much as for a fresh one). This also explains
  why a reservation probe on a fresh local is inert here: a new symbol has no
  second range to import.
- **When a residual reads as "the target has one more coloured web" and the
  loop's temp ring is one slot off, they are one lever seen twice: fix the
  phase first, then re-measure the naming edit.** A pool colour *replaces* a
  ring pop, so a naming edit measured on a base whose ring is already one
  behind shifts the whole tail one further and reads as a large regression.
  `func_overlay_060_F0000000_18B9DD8` (2026-09-11, matched): twelve named
  forms of the inner pointer had been refuted at 71-84 words and 0x68 frames.
  The phase was one phantom pop -- `&array[i << 1]` on an `s16` table, where
  the doubled index folds into the one shift and still costs a temp (L65 on a
  shift) -- and with it in place the same named local colours v1 at the same
  0x60 frame and closes the function. The 0x68 frames came from the `(u32)`
  address carrier needing a cell, not from the extra declaration.
- **uopt places a CSE'd float constant at the head of the first block that
  uses it, so which block a constant is hoisted to is a region question, not
  an emission-order one.** On `overlay1UpdateAimedTransient` (2026-09-11,
  19 -> 14) twenty forms had all emitted a `-30.0f` immediate before a trig
  import load because every form kept the three stores in one block; an
  `if (1) { }` region around the three stores, with the import read outside
  it, puts the load first and makes the arm byte-exact. A bare block is +1
  instruction; `do { } while (0)` ties; carrier identity for the call result
  still matters (three of eight f32 locals tie, the rest are worse).
- **A declared local that carries a byte offset rather than an element index
  removes the scaled-index CSE web.** `func_overlay_009_F00010B4_186772C`
  (2026-09-11, 20 -> 6): with `tableIndex` as the element index, the shared
  `<< 2` at two array reads is a uopt web (save 3.0, nocs 1) that globalcolor
  always colours, and no force reaches the target because the target's scaled
  index is a ring temp. Carrying `tableIndex` pre-scaled and reading through
  `(u8 *)` bases makes the pre-scale sum the coloured web, as the target has
  it. A ten-probe standalone TU reproduces the mechanism.

- **A counted-loop proof depends on the recurrence as well as the initial
  value; an opaque stride can avoid an opaque initializer's memory temporary.**
  When IDO rewrites a signed loop bound into a not-equal test against a hoisted
  constant, first check which part of the counted-loop proof to interrupt.
  In the controlled overlay-58 experiment, a literal zero start with
  `index += (index * 0) + 1` retains the signed comparison, then folds the
  product in ugen without a spill. Three equivalent recurrence spellings
  produce the same object; putting the outer index in the zero product brings
  the temporary back. The lever therefore depends on the expression's live
  range, not just its arithmetic value. This is disclosed inert source
  shaping, with the index initialized and its complete domain proved; it
  does not justify undefined arithmetic, observable side effects, or assuming
  the same phase ordering on another compiler. **Object identity does not imply
  identical allocator weights:** the two-read additive recurrence and one-read
  multiplicative recurrence can emit identical owned text and relocation maps
  while retaining different total savings for the index. A changed competing
  range can expose that hidden difference and select a different colour. When
  reusing an apparently equivalent source form after a shape change, compare
  its decision records as well as its object. This is an instrumented diagnostic
  lever, not permission to adopt nonexact inert source. See the
  [controlled comparisons and semantic audit](matching-triage-handoffs/func_overlay_058_F000138C_18B0574.md)
  for the bounded full-TU experiment and its coupled allocation effect.
## Definition regions, complete source shapes, and measured allocation phases

- **A missing loop-entry guard can be a definition-region effect.** Defining
  the initial count before the surrounding calculations and calls can retain
  the generic guard that a later definition lets the optimizer remove. Inspect
  where the count is defined before introducing an opaque initializer or
  sweeping the comparison spelling. This does not authorize extra observable
  reads or changes to the iteration domain. The exact
  [angle-candidate solver closure](overlays.md#overlay-1-angle-candidate-solver-closure-2026-09-12)
  supplies stock-object and ROM evidence.
- **A natural counted scan can change a competing web's save ratio.** Replacing
  a manually spelled loop with its ordinary counted form can change the basic
  blocks used in the allocation denominator while retaining the actual loop
  bounds. A tied ratio then lets web order choose the winner. Price the colour
  exchange with an accepted direct force, compare the recorded denominator,
  and re-prove every stack home. Dynamic iteration counts are not that
  denominator, and a frame-growing candidate is not an improvement merely
  because its positional score falls. The exact
  [grid-mask closure](overlays.md#overlay-35-grid-mask-builder-closure-2026-09-12)
  demonstrates the mechanism and its frame constraint.
- **Reconstruct a complete SDK command sequence before pricing its pointer
  colours.** A single native macro among manually expanded commands can leave
  a separate command-pointer web; using the existing macros for the complete
  sequence can restore both allocation and emission order. Keep custom commands
  explicit and retain provenance for the actual header. This extends the
  source-line grouping observation above; it does not imply that any isolated
  macro replacement improves a match. See the exact
  [overlay 26](overlays.md#overlay-26-draw-groups-closure-2026-09-12) and
  [overlay 29](overlays.md#overlay-29-draw-groups-closure-2026-09-12) closures.
- **Trace allocation phases instead of inferring them from call presence.**
  Call-free code can still execute both the save-ranked and remaining-web
  allocation phases. The exact grid-mask TU has both phases in its correctly
  identified procedure; the instrumented object passes section, relocation and
  symbol fidelity against the same configured stock input. Thus absence of
  calls alone does not establish ascending-web allocation for the entire
  function. Resolve the procedure ordinal and inspect its decision records
  before applying a phase-specific law. Statement order can still change
  emission and scheduling. Trace output and compiler diagnostics
  remain private; stock output and the normal promotion proofs supply credit.

- **An index type can partition a shared array-address range.** When two
  regions share a strength-reduced array cursor, changing the subscript to an
  unsigned expression in just one region can separate their allocation ranges.
  In a controlled full-TU experiment, that split changed the occurrence-block
  denominators and lowered each range's save ratio below competing address
  constants. Giving both regions the same unsigned spelling rejoined the
  ranges and removed the benefit; several equivalent full-width expressions
  reproduced it. Inspect the allocator records and prove stock-output fidelity
  before attributing the change to expression cost or declaration order. The
  conversion is valid only for a proved index domain, and a smaller positional
  residual can still worsen aligned naming differences. This is a reproducible
  diagnostic lever, not an exact-match claim or a universal unsigned-index
  preference. See the
  [typed-index partition and retained tradeoff](matching-triage-handoffs/func_overlay_058_F000138C_18B0574.md).
- **Naming an intermediate changes both allocation and the home census.**
  A named pointer or floating intermediate can replace an expression temporary
  and restore the target's register use, while also adding a declared stack
  home. Measure those effects separately: first confirm the local allocation
  improvement, then use the frame census to place the new home and account for
  any existing unused workspace. Definition placement can independently change
  saved-register ranking even when the value stays live across the same calls.
  The exact [overlay 98 object check](matching-triage-handoffs/overlay98CheckObject.md)
  required all three effects together. This is not permission to remove live
  storage or reorder observable operations; check expression widths, floating
  association and workspace use, and require stock, relocation and linked ROM
  proof. A colour-force win alone establishes none of those source properties.

- **An indexed scale load can outrank a conversion even when a float cast is
  inert.** If an inline multiply has the correct operations but evaluates its
  memory operand too early, compare the indexed access with a scalar struct
  member at the same proven byte offset and load type. This changes operand
  weight without introducing a float carrier; a same-type cast need not do so.
  Verify the data layout, unsigned conversion domain, aligned residual, and
  linked relocation identities. It is an access-shape lever, not permission to
  relabel unrelated storage or a universal preference for structs. See the
  [member-scale source closure](matching-triage-handoffs/overlay89InitializeEffect.md).
- **A region can cross a caller-versus-callee cost threshold without changing
  the call set.** When the desired caller register is offered but narrowly
  loses to the first saved register, inspect both prices before changing save
  ratios. In a controlled exact source closure, an inert control-flow region
  raised the first-saved-register toll while leaving caller cost and weighted
  reference count unchanged; the desired bank then won naturally. The changed
  component count was not itself the pricing mechanism. An ordinary nested-if
  spelling was inert, so not every brace or equivalent conditional supplies a
  region. The lever also cannot raise a toll already at its saturation limit.
  Any retained inert form still requires independent semantic review,
  disclosure, cleanup-queue coverage and unmodified-compiler promotion proof.
  See the [cost-table and source receipt](matching-triage-handoffs/overlay89InitializeEffect.md).

- **Move a definition into its consumer before assuming its carrier must be
  deleted.** A selector argument computed in a preceding statement and the
  same assignment embedded in its call argument can retain the named value
  while changing the order of temporary subtrees. In an exact closure, moving
  the assignment preserved the draw-register sequence and fixed the call
  window; replacing it with a plain argument instead added a draw and changed
  the ring. A comma expression that only moved a dimension's source attribution
  was byte-inert on a separate constructor, so grouping alone is not the lever.
  Require no intervening observable operations, no unsequenced accesses to the
  assigned local, and unchanged argument types and values. Census totals also
  include source-location directives: fewer emission records need not mean
  fewer machine instructions, and equal per-line counts can hide exchanged
  operations. Use retained aligned object deltas and stock/relocation/ROM proof.
  See the [call-site assignment closure](matching-triage-handoffs/overlay19BuildAdjacency.md)
  and the [constructor context controls](matching-triage-handoffs/overlay34CreateRecord.md).
- **Guard membership can repair a generated cursor's temporary-allocation
  tradeoff without changing stack homes.** When a source form reaches the
  wanted cursor allocation but loses a cached local load and a later shared
  global address, inspect the local initialization's position relative to the
  existing nonempty-loop guard. Moving a local assignment into that block can
  change the first-use allocation and remove the extra temporary draw while
  preserving the frame and transition-address reuse. Prove that the local is
  unused on the skipped path, then compare source-line draw counts, aligned
  rows and stock output in the complete TU. In the controlled
  [guarded-row and cursor-sharing experiment](matching-triage-handoffs/func_overlay_058_F000138C_18B0574.md),
  the move was inert on earlier cursor shapes, and named transition values
  did not substitute for it. This is a context-dependent definition-placement
  lever, not permission to remove an observable initialization or a claim
  that every guard move improves allocation.

- **Re-measure line scheduling after changing which indexed values have named
  carriers.** An explicit byte-offset local and an explicit loaded-value local
  can together preserve an unwanted allocation shape. Replacing both with
  typed indexed accesses lets the compiler form those values itself; controls
  that remove only one carrier do not establish the combined form's behavior.
  In the [indexed-resource source closure](resident.md#2026-09-13-func_80006448-indexed-resource-release-lane-f1),
  the combined edit repaired the call windows, and joining the loop initializer
  to the loop's physical source line then repaired the remaining entry order.
  The draw sequence was unchanged throughout: equal draws do not imply equal
  allocation or final assembler scheduling. This extends the line-barrier
  observation above with a stock-compiler source closure; the allocator's
  internal cause remains unproved. Preserve index bounds, access
  widths, and observable load/call order when substituting indexed reads;
  repeated accesses are not interchangeable across volatile reads or mutations.
  Retain aligned object comparisons and require exact relocation and linked-ROM
  proof before adopting either edit. A line fold on an unchanged source shape
  can remain inert.


- **Test a countdown's snapshot separately from its truth spelling.** When a
  guarded loop has a surplus boolean instruction or lacks a counter copy,
  compare the explicit zero comparison, scalar truth, and local postdecrement
  guard as separate changes. In the
  [boundary-chooser closure](matching-triage-handoffs/overlay2ChooseBoundary.md),
  paired guard changes repaired an insertion/deletion pair while leaving the
  complete draw sequence unchanged. A later countdown guard supplied an extra
  draw at unchanged instruction extent and repaired the following ring phase.
  This is why neither final word count nor the number of source operators is
  a temporary-allocation census. Retain per-line draw differences and aligned
  object comparisons together. Replacing a memory retest with the captured
  count requires proof that no intervening operation can change that memory;
  keep the counter width, iteration count and defined decrement domain. The
  result does not justify replacing arbitrary loop conditions or adding dummy
  work, and promotion still requires untouched compiler output, relocation
  identity and linked-ROM proof.

- **Choose which value is named before tuning its register.** A nested
  unsigned calculation can consume separate temporary draws even when its
  final value is already assigned to a local. In the
  [relocation-value closure](matching-triage-handoffs/overlay14LoadRelocatedValue.md),
  naming the masked-and-tagged intermediate before adding the base reduced
  draws. Reading through the advancing cursor while retaining the old pointer
  only for stores then changed allocation without changing the draw sequence.
  Separate those experiments: fewer draws and different allocator weights are
  distinct observations. Preserve unsigned widths, arithmetic association,
  access identity and read/write order; this does not authorize reassociating
  floating-point expressions or substituting aliased/volatile accesses.
  Once only stack displacements remain, revisit declaration homes: coloured
  locals preceding the deepest spilled local can enlarge the frame, so total
  local count alone does not predict frame size. Confirm the complete TU,
  relocations and linked ROM before adopting any spelling.

- **A direct memory retest can remove a named web while adding temporary
  draws.** In the
  [timer-update closure](matching-triage-handoffs/overlay14PrepareInputState.md),
  replacing an assigned scalar snapshot with a direct update and retest added
  draws on the intended lines without adding emitted instructions. One removal
  barely changed the residual; removing its paired carrier changed which
  registers belonged to the temporary ring and repaired earlier windows too.
  Source-level carrier count, draw count and final instruction count therefore
  measure different things. Compare each change's line census and aligned
  windows, then measure the combination rather than inferring independence
  from two small single-edit scores. This substitution requires no intervening
  mutation or observable reload, matching access widths and the same defined
  arithmetic domain. Do not use it for volatile state or across a call. Exact
  untouched compiler output and runtime relocation identity remain mandatory.
- **Trace split admission when address formation and draw counts agree.** A
  shared address can lose reuse even when its initial range has identical
  membership, uses and savings. In the
  [controlled address-splitting comparison](matching-triage-handoffs/func_overlay_058_F000138C_18B0574.md),
  an additional induction carrier changes which adjacent blocks enter a greedy
  split. In the observed split mode, admission tests both new shared
  interference against the old available-register count and combined
  interference against twice the new available-register count. Rejecting one
  block restores its tentative forbidden-register changes; that can make a
  later block admissible and strand an address use in an unprofitable fragment.
  Trace membership, interference and available-register counts through the
  entire component before choosing a source lever. Reducing the seed count
  alone did not help: a guarded pointer or fresh index added replacement
  ranges at the next block. Definition placement must remove the relevant
  overlap across the component, not merely relocate it. Follow successive
  splits of the original range too: removing one captured scalar repaired the
  first component's admissions but made a later unprofitable component admit
  the same address use. Its source edit preserved the complete draw sequence
  and frame traffic yet still lost reuse. Uniform interference removal can
  therefore satisfy one component's bound while violating another's; look for
  definition placement that changes overlap at the required component only.
  An arithmetic exclusion using fixed block membership and availability is
  limited to that measured topology. This comparison proves the split
  mechanism, not a successful source recipe or exhaustion of source space.
  Preserve per-site draw constraints and require stock/instrumented fidelity;
  other split modes and source shapes need their own measurements.

- **A folded argument expression can retain a reset's emission position.**
  When a reset is already an independent ready instruction but executes
  before a call's delay slot, inspect its order among the zero-height
  argument definitions. In the controlled
  [reset-slot experiment](whale-gap-emission.md#the-reset-reaches-the-slot-lane-wv-i),
  a discarded comma assignment in the last argument is emitted before the
  register arguments. Making that argument's value depend on the assignment,
  as in `255 - (index = 0)`, retains the reset after those definitions even
  though the arithmetic folds away. Its dependency fields remain unchanged;
  the new ready-list order leaves it for the slot. A separate small switch-arm
  fixture reproduces this behavior, while otherwise similar function-entry
  fixtures do not, with either int or ABI-width long locals. Treat the form as
  a context-dependent source lever, not an evaluation-order guarantee.
  Require faithful generator/scheduler evidence and final instruction identity;
  unchanged draws alone prove neither position nor width. The assignment must
  initialize a nonescaping local, have no conflicting access in another
  argument, preserve every argument value, and use defined arithmetic.
  Duplicating a later definition can still delete the early reset, and a
  separate induction introduced to retain it can still lose later address
  reuse. This lever does not solve either of those lifetime constraints.

- **A local assignment after a call can reach a slot that its before-call
  spelling misses.** Symptom: an independent constant definition precedes
  the call's argument setup, while the target puts it in the call slot.
  Lever: place the assignment immediately after the call when the local's
  address does not escape, no argument reads it, and no intervening use is
  crossed. The compiler can move the definition back into the call slot with
  a different argument order. A controlled full-TU experiment changes only
  three executable words, all to target values, with identical extent and
  temporary draw order; an argument relocation moves with its instruction.
  This is a measured source-order lever, not a universal scheduling promise
  or authority to move global/volatile assignments across calls. See the
  [portrait-slot control](whale-address-reuse.md#the-banked-three-word-repair).

- **uopt closes a straight-line block when `varrefs` reaches twenty, counted
  on cfe `Ulod` of locals, not on uopt CSE.** Symptom: a long field-init
  sequence splits between two stores the target emits in one block, so a
  literal's `nocs` or a store pair is the residual. Mechanism: `getop`
  sets `endblock` when `varrefs >= curvarreflimit` (default 20) at a
  statement boundary (`ustack` empty, not in a call). `varrefs++` fires on
  each `Ulod`/`Uisld` of a non-`veqv` isvar (stack locals and parameters).
  Forwarding an `assigned_value` skips the increment; ILODs, constants and
  the store opcode itself do not increment. The statement that reaches 20
  stays in the block; the next statement opens the next one. Lever: count
  cfe local-loads in the Ucode of that block. A 2-unit store that must
  share the block with a following 1-unit store needs at most 17 units
  before it. Limits: uopt CSE of the same isvar does not reduce `varrefs`
  (each cfe `Ulod` still counts). Copy-prop does not replace a later
  pointer-field `Uistr` of a named s32 whose assignment contains an ILOD,
  so splitting `width = p->w; width = (width - 1) << 5` does not make
  `p->f = width` cheaper. Repeating the ILOD expression at each store
  rematerializes under aliasing. Chaining `p->f = x = expr` is the
  folded-def family and moves the frame or reloads a truncated field.
  Combining adjacent zeros as a wider store saves a unit in the counter
  and changes the opcode. A post-call copy of the pointer into another
  local (`p = q` with no ILOD in the assigned tree) is the form that
  *does* skip the base `Ulod`s: later field stores through `p` do not
  increment `varrefs`, so a fill that spent the budget at one store can
  keep the next store in the same block and emit the target's order.
  See the
  [constructor block-budget receipt](matching-triage-handoffs/overlay34CreateRecord.md).

## Adding a learning

Add a short entry only after the result is reproducible. Cite the durable
Mickey evidence, state where the lever does *not* apply, and prefer updating
the upstream decomp-workbench field guide when the behavior is toolchain-wide.
A one-function curiosity belongs in that function's handoff, not here.
