# 0032. Source-authenticated mixed callback DATA

Status: Accepted
Date: 2026-10-26

## Context

A genuine action table combines two function-pointer fields, a halfword mask,
NULL fields and ordinary record padding. Pure ordinary DATA rejects relocations;
the older pointer-input cases cannot establish private text function targets or
mixed scalar fields. Selection or an old relocation site cannot establish these
roles. A first stock source input also demonstrates distinct compiler allocation,
ELF symbol span and C object extent.

## Decision

Add the explicit typed-callback-records-v1 source profile, preserving every
existing profile and default refusal. The first closed grammar is one mutable
positive-literal-bound array of complete records: a signed-long-o32(void)
function pointer, a void(void) function pointer and an unsigned-short mask.
Require complete initializer rows, direct uniquely declared compatible function
bases or canonical NULL, and bounded C-defined unsigned literal OR/shift masks.
Reject extra storage, filler fields, qualifiers, packing, attributes, unsupported
expressions and assembly. No per-module name, coordinate or site list is proof.

Recover the actual Make compiler vector and wrapper frontend, capture the actual
IDO preprocessed source, and reuse the pinned parser. Compile that same frontend
with zero-storage bitfield-width checks for the derived array size, record size,
function-pointer width and halfword width. Require all meaningful allocated
bytes, REGINFO, original symbol indices/roles/sizes and ordered relocations to
equal the untouched compiler output. A measured false-size control must fail:
negative array typedefs are insufficient because this compiler only warns.

Keep the real symbol size and compiler allocation unchanged. A prepared view may
change only the DATA section header to the independently compiler-proved C end.
Require a zero compiler-alignment suffix shorter than sixteen bytes, one actual
positive GLOBAL owner and no extra objects or COMMON. All original ordered R32
sites must exactly cover the source's non-NULL pointer fields, with original
undefined GLOBAL FUNC indices, zero addends and full field bounds. The reviewed
first input has reported symbol66, allocation80 and proved C extent72; these
remain separate facts. Its60 field bytes and12 record-padding bytes are not
72 semantic-field bytes.

Declare the ordered ordinary same-module C text-input graph. Ordinary GNU ld-r
with the current SUBALIGN4 policy derives a function-bearing prefix without
zero bindings. Retain its original inputs, bytes, symbols, relocation order and
map. Discard only explicitly checked nonallocated debug metadata. Any remaining
ordinary ASM text inputs are explicitly held outside-prefix compatibility,
including assembler-generated FUNC labels; they never supply a callback target
or C ownership. No ASM input or omitted C contributor may interrupt the prefix.

Authenticate each callback against fresh actual source CPP and a sized allocated
GLOBAL FUNC in the original and configured input, exact target/TEXT bytes and
geometry, the ordinary prefix definition and the unique current final input/map
and function. This is limited FUNCTION/TEXT evidence; unrelated configured
RO/normalization remains held compatibility and is not claimed as replayed.
Private same-module callbacks use the actual text base; declared exports retain
their actual SYMBOL rows. No fake ORT export or bare ABS/name decoding is allowed.

Replay the whole prepared DATA input through the ordinary linker and compare all
scalar, NULL, padding and linked pointer bytes. Derive every identity before
comparison with the complete runtime record set. Missing records never backfill.
Reuse the existing capture/loaded-byte holds, atomic inventory, Resolver epochs,
warm invalidation, end-pack and marker-last publication checks. Failure exposes
neither stale nor partial authority; restored inputs require fresh success.
Private capture paths are live holds, not volatile Make dependencies.

## Consequences

A source/module proof and normal ROM, runtime, map and required tooling gates
must precede adoption. This helper alone adds no initialized/executable credit,
native or whole-image source-only claim. Other callback signatures, cross-module
or resident functions, object pointers, arbitrary expressions and other shapes
remain unsupported. Original capacity, reader bounds and callback lifetimes are
separate source-contract questions.
