<!-- plateau-handoff:overlay15MoveStars:start -->
### `overlay15MoveStars` plateau handoff

- source: `src/overlays/o015/overlay_015.c`
- score: 0/54 words, promoted
- frame: 0x40
- relocations: 21
- first mismatch: none
- summary: Matched. The field is a locally-defined static struct reached through a pointer taken at function entry, so the bound reads are direct and share high halves.

#### 2026-10-01, lane b-o101: matched; the shared high half has a natural source form

Promoted at 54 of 54 words, delta 0, frame 0x40, 21 of 21 relocations.

The reopen condition asked for a natural direct-load form that shares one
high half per adjacent bound pair. It exists and needs two things together.

The storage is one struct per particle field, defined in this unit: twelve
bound words, the movement vector at 0x30, a second vector at 0x3C and a
pointer at 0x48, 0x50 bytes, two of them filling the 0xA0-byte BSS. as1 only
shares a high half between two references to the same locally-defined symbol
that fall in one aligned eight-byte pair, which is the pairing the target
shows: 0 and 4, 8 and C, 10 and 14, 18 and 1C, then 20 alone.

The access is through a pointer local initialised to the struct's address at
the top of the function. Measured on mini units and then here: where the
pointer is used in the block that assigns it, uopt keeps it as a base
register, which is the three movement stores; where it is used in a later
block, or after a call, uopt forwards the address into each access as its own
constant, so nothing is commoned into a base register and each read is direct.
Naming the struct members without the pointer gives one base register for all
nine reads, twelve bytes short; nine separate scalars, extern or static, give
nine high halves, sixteen bytes long. Both were the recorded dead ends.

Because the struct is static, each access is a section-relative record whose
addend is already the shipped module-relative offset. The overlay's runtime
table owns those records, so mk/overlays.mk drops the sixteen static ones with
filter_elf_relocations.py, as overlay 57 does, and adds the placeholder rename
for the resident starfield mover.

The same form takes overlay15UpdateMovingStars from 84 words and 28 bytes long
to 0 and overlay15DrawRain from 13 to 0 on the fast loop; those are separate
commits.

Gates: gmake verify, check-overlay-syms and promotion-proof pass.


#### 2026-09-11, lane `lane/p4-xfer`: refuted, and the size delta is now fully accounted

Unchanged at 30 relocation-masked words, size delta +16, frame 0x40, three
relocation-artifact words. Aligner: 37 byte-exact, 4 register naming, 3
immediate only, 15 really different. No edit adopted.

**The transfer from `overlay15DrawRain` is refuted because its precondition is
already satisfied here.** DrawRain's structural bucket fell 12 to 3 by
dissolving a padded struct into separate extern scalars. This sibling's nine
bounds are already nine separate extern scalars. It is not a function the
mechanism can move; it is already in the state the mechanism moves a function
toward, and what is left is precisely the defect the mechanism could not fix in
DrawRain either.

**The size delta is now fully accounted.** The ROM spends five overhead words
on the nine bound loads and this candidate spends nine. Four words is sixteen
bytes, which is the whole delta; nothing else contributes.

**The cost model, which explains why an earlier declaration sweep read flat.**
A symbol referenced once costs one overhead word, because the low half folds
into the load's displacement. A symbol referenced twice or more makes uopt
build a base register, costing two. So four pair aggregates plus one lone
scalar also costs nine, exactly as nine singles do. That sweep was flat by
construction, not by accident, and repeating it is waste. The reachable span
is eighteen words for nine singles down to eleven for a single aggregate over
all nine, with the ROM at fourteen; the single aggregate measures 42 masked at
delta -12, three words short.

**The ROM's overhead count is reachable, and reaching it does not help.** All
nineteen contiguous partitions of the nine arguments satisfying two per group
plus one per single equals five were enumerated and measured. Every one closes
the size delta to exactly zero. None improves masked; the best is 34. Closing
the size actually moves the object further away: byte-exact falls 37 to 30 and
really different rises 15 to 23. A lane that treats the size delta as the thing
to fix here will make the function worse.

**The decision variable, restated correctly.** The DrawRain note describes this
as as1 sharing a high half between two absolute loads. That is wrong on both
halves: as1 never shares, and the ROM's shared base is not a peephole artefact
-- it stays live across roughly fourteen scheduled instructions. This is a uopt
address-constant decision, and the variable is uopt's choice between an
assembler-temporary shared high half costing one word and an allocatable base
register costing two. Every spelling reachable from declarations, groupings and
argument order picks the base register. Register pressure is the obvious
untested hypothesis and is where the next pass should go; do not spend another
pass on declarations, groupings or argument order.

#### 2026-09-13, lane l1: generated scale differential

Fresh baseline: 58 candidate words against 54 target words, delta plus sixteen,
frame 0x40, thirty masked differences. Alignment is 37 exact, four naming,
three immediate and nine paired structural rows, plus five candidate-only
and one target-only words. Procedure 3 has fourteen draws and 73 emissions.
The prior scalar/aggregate and exact-size partition receipts were read first.

One L160 control removes the named float scale and spells the same rate
conversion at each of its three multiplications. The intended mechanism is
release of the declared carrier before the bound-load call block. It removes
one source-location emission at the former definition, but every line's draw
count and the complete draw sequence stay fixed. Stock text is byte-identical
and every aligned window and gap remains unchanged. Thus deleting this carrier
does not change allocation pressure or reach the shared-address lowering.
Named Ucode mapping and complete stock/capture fidelity pass for both forms.

Retain the original guarded body. Stop early under ADR 0018: this new carrier
control is byte-inert and the prior grouping/argument-order controls already
bound the address-lowering problem. Repeating those partitions cannot provide
new evidence. Next action still requires the target's shared high-half form
without a full materialized base. Sources, objects, censuses and differences
are preserved under ignored build/l1/overlay15MoveStars. Commands: configured
compilation, allocator_trace_receipt mapping, draw_census differences,
residual_map comparison, finalize_plateau and tools/gates.sh. No byte credit.

<!-- plateau-handoff:overlay15MoveStars:end -->
