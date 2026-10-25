# 0030. Source-backed encoded-address linker values

Status: Accepted
Date: 2026-10-25

## Context

A target32 command word can encode a real object's runtime address without
being an object-interior pointer. Controlled stock IDO/o32 compiles kept
explicit address arithmetic in text, whereas a DATA initializer folded it
into R32 but allocated storage. An address-only linker carrier instead emitted
one zero-addend text HI/LO pair; ordinary GNU ld supplied the derived word.
Owner movement, independent DATA-origin movement and signed-low carry controls
preserved the real input and reproduced the ordinary relocation fields.

The [GNU ld Source Code Reference](https://sourceware.org/binutils/docs/ld/Source-Code-Reference.html)
documents symbols without allocated memory and array-style extern declarations
used as addresses. Actual IDO output establishes this measured carrier case;
the manual does not establish original source, IDO behavior generally or native
portability. A fixed numeric alias also passed baseline arithmetic, proving
that equal values alone do not authenticate an expression's source dependence.

## Decision

Accept a closed target linker-value interface, initially only the GLOBAL O73
polygon producer's direct `Overlay73Command.w1` u32 store and the real writable
DATA owner `o73Triangles`. **This contract does not admit production. Existing
guards must continue to refuse the new role until every required control below
is implemented and passes.** A mechanism-positive diagnostic is insufficient.

Use an address-only `extern unsigned char marker[]` carrier and a direct unsigned
32-bit conversion followed by its value-preserving unsigned 32-bit field store.
The actual O73 macro casts to `unsigned int`; its `u32` field is `unsigned long`.
These distinct C types are both 32-bit under the authenticated IDO/o32 flags. Do
not change the macro or assume typedef identity to satisfy a proof predicate.
This is an explicit target linker-symbol convention, not a C
array definition or storage claim. Permit no value load, dereference, subscript,
pointer arithmetic/escape/round trip, callable use, R26/R32 or extra field addend.
The marker owns zero bytes and supplies no capacity or object bounds. Observe
its actual compiler/final types: the measured UND GLOBAL OBJECT size0 becomes
ABS GLOBAL OBJECT size0. Do not assume NOTYPE, retag symbols or fabricate storage.

Let O be the real owner's module offset, T the actual runtime DATA origin and B
the runtime load base. The ROM-link scalar is W=(O-T+2^31) modulo32; the source
interface after normal LOCAL relocation is E=(B+T+W)=(B+O+2^31) modulo32. Keep
owner identity, stored scalar and true section boundary distinct. A biased word
is not a two-gigabyte interior pointer or an ordinary resident-address API.

Produce W with one fixed-form ordinary-link expression over the actual owner
and genuine DATA origin in that link's context. Authenticate the expression's
structure and referenced definitions independently; a declared association,
name, JSON witness or numerical equality is not proof. Never derive it from
shipped operands, fixed addresses or site lists. Current target-derived alias
synthesis is not this producer. Keep the real owner allocated and unshadowed;
never create a fictitious ORT object/function or use a canonical fallback.

Freshly bind the real C owner to actual Make/flags/CPP/include lookup, original
compiler index/type/extent, complete input bytes, configured reproduction and
unique nonoverlapping map ownership. Require a positive-sized GLOBAL writable
DATA object; reject ABS, COMMON, unsized, retained-assembly and wrong-role
substitutes. Bound the full owner within DATA and preserve compiler alignment
separately. Authenticate every effective-source carrier use, its actual u32
field and GLOBAL function, and the complete original zero-addend HI/LO pair.
Missing/extra uses, shadowing, unsupported operands or source/raw disagreement
must refuse rather than inherit ordinary object or integer-word authority.

Preserve untouched compiler instructions, original indices/addends, complete
function geometry and ordered references. Only ordinary ld may evaluate its
relocations; independently replay its exact script/command/input closure and
verify every nonrelocation bit. No operation may be inserted, deleted or patched
to repair a candidate. Read back the actual final marker's value and zero extent,
real owner/map, body and complete ordered runtime records. Independently decode
the signed-low pair and require T+W=O+2^31 modulo32, with complete pair coverage.

Reuse existing capture/seal/provider lifecycles. Hold source/CPP lookup, loaded
original/prepared/configured observations, tools, expression, actual link inputs,
final ELF/map and role evidence through use, warm reuse and publication. Bind
loaded parses to receipt hashes. A failure clears derived inventory and old
Resolver authority and withholds the completion manifest; recovery requires
fresh validation. No stale compatibility fallback or partial publication.

Require actual production, cold/warm, owner/origin movement and carry controls;
reject numeric coincidence, owner substitution, wrong section/origin/bias,
unsupported use/pair edits and late source/object/parsed/script/readback mutation.
Old numeric authority must be removable or poisonable without supplying the new
result. Ordinary ld accepting a counterexample is not an admission-check pass.

## Consequences

This adds a distinct contract to ADR0021's ordinary-link boundary without
relaxing ADR0001/0002 or the genuine-span limits in `ido-linker-spans.md`.
Implementation and full natural source/module/ROM/identity/manifest proofs are
still required before promotion. O13's separate owner and LOCAL function are
not admitted by analogy; do not change binding or move storage to evade guards.
Original marker usage, native portability, physical-address domain, graphics
lifetime and original capacity remain unproved. No new storage/executable credit
or general source-only/relayout/native completion follows from this decision.
