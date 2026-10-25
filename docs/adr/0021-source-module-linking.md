# 0021. Link complete source modules with runtime section-relative addresses

Status: Accepted
Date: 2026-10-23

## Context

The runtime loads a module's initialized text, data and read-only data as one
body. Its stored operands use offsets relative to their own input section;
runtime relocation records supply the final module or exported-symbol base.
A conventional host link that assigns every input section one combined VMA
changes those operands. Retaining a binary pool and externalizing the compiler
pool hid this distinction while text was being reconstructed.

The overlay 24 pilot has exact compiler-emitted text and a literal pool that
matches the retained initialized data. Source ownership should remove that
retained pool without changing function bodies or introducing offset patches.

## Decision

Allow an explicitly selected complete translation-unit module to pass through
`tools/link_overlay_module.py`. Preserve the original compiler object. Prepare
only symbol and relocation metadata: undefined references must name declared
ORT exports, and references to an exported definition in the same object use
a separate zero binding while retaining the actual definition. Private targets
retain ordinary local linking. Unknown references fail closed.

Use ordinary GNU ld to link `.text`, `.data`, `.rodata` and `.bss` independently
at zero, with initialized load addresses concatenated in runtime body order.
These zero bases represent the actual stored runtime operands, rather than
substitute addresses used only to obtain a diagnostic score. The serialized
relocation tables are subsequent standalone ROM segments, outside the body's
loadable sections and BSS extent.

Retain the ordinary linker output, explicit bindings, script and command.
After localizing external zero definitions, convert its ELF container from
ET_EXEC to ET_REL by changing only ELF header type, entry and program-header
metadata. All section payloads and symbol/section tables remain unchanged.
The final ROM link consumes this container. This conversion models the
original module format; it is distinct from per-function instruction editing,
blanket unresolved-symbol suppression or hand-authored relocation-site lists.

## Acceptance and limits

The source-module promotion route freshly compiles the configured ordinary C,
replays the actual module linker command with pinned tools and export names,
and authenticates compiler, prepared, linked and converted objects. It checks
the configured allocated sections and symbols against that reproduction.
Source section ownership and canonical exported definitions independently
resolve every original module relocation; the shipped site never supplies its
identity. Require exact count, type, offset and identity against all runtime
records, exact initialized module bytes in both linked ELF and ROM, and exact
BSS extent. Keep configured relocation count zero explicit; do not relabel a
stripped object as retained compiler relocation evidence.

A cross-overlay BSS export requires a unique global definition in actual
nonempty allocated writable NOBITS storage, with the linked module bytes and
section capacity authenticated against its runtime header. Translate the
symbol's offset within that section onto the runtime text-plus-data prefix;
an older host layout may place serialized tables before the BSS section.
Neither an encoded symbol name nor an ABS assignment supplies that identity.
An explicit physical loader reserve can support such an unsized export while
its unknown contents remain borrowed; it earns no reconstructed C ownership.

Existing function geometry, provenance and full-ROM gates still apply. No
instruction bytes may be edited to obtain a match. The linker may perform its
ordinary relocations on untouched compiler instructions. The initial selection
is overlay 24; incomplete, multi-object or otherwise unsupported modules remain
on the existing flow until their own evidence proves this route. The raw
compiler objects remain the packer's relocation authority during this staged
transition, and remaining borrowed layout or table residuals stay explicit.

A separately selected complete data-only input can use the same linker for
its declared ORT imports or the bounded same-owner LOCAL prefix below. The
overlay 8 tuning-pointer tables establish the ORT case: no instruction or BSS
payload, one complete genuine initialized input,
and a separate artifact directory for that source. Require fresh untouched
compiler output, the ordinary helper receipt, a unique final-map contribution,
and runtime identities independently anchored to current source-owned export
definitions. Keep the twenty original compiler R32 relocations explicit; the
configured object has zero relocations after linking. Hash the actual recipe,
helper, export names and tools when recording this proof. This does not relax
the complete main-module promotion guard or authorize incomplete-body padding,
section trimming or handwritten relocation-site bindings. Producer discovery
admits only complete data-only inputs that pass the supported-case guard below;
other initialized inputs and their metadata borrowing remain explicit.

A complete data-only aggregate at the module's initialized-data prefix may
also retain same-input LOCAL R32 references. Require one sized global object
owning the entire input, bounded addends to that same definition, a unique
matching final definition and map contribution, and the actual runtime data
base derived from current module geometry. Independently compare every source
site, type, order and target with the LOCAL runtime records. Missing a pointer
relocation must fail even when replacing its zero-offset target with NULL
leaves all configured bytes unchanged. Other section groups and cross-input
LOCAL targets remain unsupported by that case; fresh capture and ordinary
helper replay still apply to the complete input.

An explicit held Make `LOCAL_DATA_PREFIX_OBJECTS = 1` additionally admits two
genuine adjacent positive-sized GLOBAL OBJECTs covering one complete DATA-only
prefix, initially the reviewed frame and count/list arrays in overlay 59.
Preserve both actual compiler definitions and original symbol indices; reject
extra allocated definitions, gaps and suffixes. Both owners must contribute
original ordered aligned R32 records. Every complete four-byte site must belong
to one owner, and every target must name the first actual compiler object with
an aligned full four-byte target span inside it. Join both definitions to their
unique final allocated GLOBAL definitions and the same whole-input map before
deriving identities from current geometry and raw addends. Only then compare
the complete runtime set; missing records never backfill, even for unchanged
zero pointer bytes. The default single-owner rule remains unchanged.

Hold the explicit selection through warm revalidation and final publication;
failed validation, including a late end-of-pack or publication hold, clears
derived selection and producer/definition inventory. Existing Resolver epochs
refuse their cached names, ORT identities and row groups until revalidation.
Recheck private capture holds immediately before the completion marker. Bind parsed
fresh/configured/replayed ELF bytes to the capture/helper receipt digests, and
hold the loaded ordinary raw observation through end-of-pack checks. Fresh
artifact holds do not become volatile build dependencies. Genuine C field and
initializer review uses the current configured recipe and dependency/include
context; this case does not claim a separately sealed compiler-CPP/type proof.
The original capture and all its internal checks share their unchanged bounded
operation deadline. Later hold revalidation gets its own bounded operation,
with one shared deadline for recipe and dependency traversal. Capture age is
not a proof expiry; this does not renew a running capture or skip changed inputs.

The packer's supported-case guard freshly compiles and replays
selected pure ORT-pointer inputs and bounded same-owner LOCAL prefixes,
authenticates their C definitions and unique map contributions, and refuses
any source/runtime identity disagreement.
Warm metadata dependencies include those raw/source/provider/header/tool inputs,
so unchanged zero pointer operands cannot hide a changed source target. Other
LOCAL, grouped and implicit-literal inputs remain unsupported by this guard.

Publish authenticated data-only producer objects only after every source,
provider, map and tool hold succeeds. Revalidation clears prior object and
definition caches before rebuilding this inventory; failed or recursive
discovery cannot expose a partial or previously accepted inventory. The
ordinary raw paths remain dependencies while their complete surfaces are
authenticated against fresh compiler output. Undefined imports on these
inputs carry the actual proved provider identities into ordinary relocation
packing, without interpreting symbol spellings as addresses. The initial
producers are overlay 8's twenty ORT pointers and overlay 59's forty same-owner
LOCAL pointers; their sixty records replace hybrid borrowing without changing
the remaining borrowing or claiming complete source-only generation.

ORT identities for those admitted undefined R32 imports also use their held
actual provider identities. Numeric coordinates in their names are not the
authority. Namespace caches follow the published inventory's lifetime:
successful revalidation invalidates cached identities, row groups and resolved
targets; failed or in-progress validation cannot return old cached authority.
Missing or conflicting held providers fail instead of falling back to name
decoding. Other ORT rows and general name resolution retain their existing
compatibility behavior until separately proved.

A generated function proxy can use its exact suffix-free global definition
without decoding coordinates from that spelling. Require a unique sized
STT_FUNC in actual allocated executable raw text, a bounded extent, its unique
source-text map contribution and a matching sized global function in that
module's linked output. Ambiguous, nonfunction or stale geometry fails instead
of falling back. Genuinely absent definitions retain the visibly distinct
`generated-proxy` compatibility route; the initial closure changes nine records
to `source-function-proxy` and leaves seventeen on compatibility authority.
This establishes target identity, not fresh source-compilation acceptance or
new matching credit; ordinary builds and existing function proofs still apply.

Resident generated names can likewise use an actual current qualified function
before coordinate decoding. Require a unique GLOBAL sized STT_FUNC in a real
resident source C/assembly executable text input, a bounded word-aligned extent,
zero-based relocatable text, its unique nonoverlapping map contribution and a
matching canonical executable function with the same home and size. Derive the
home from that input definition and placement, never from numeric name fields.
A present qualified function with missing, ambiguous or stale input ownership
fails. Data, BSS, ABS, unsized and genuinely absent qualified definitions retain
their explicit compatibility routes; this does not establish their ownership.
The input index follows the existing provider inventory epoch. Actual loaded
resident object, canonical ELF and map bytes join the existing held-input
snapshot, including conflicting rereads and an end-of-pack hold check. These
paths are already metadata dependencies. This establishes compiled definition
identity, including assembly providers, not new source-compilation acceptance,
matching credit or zero-oracle authority.

Producer capture pins its actual bootstrap/final ELF-map pair and ordinary
link command, source, headers, recipe and tools before and after reproduction.
It cannot require the generated final-metadata output it is producing. Its
explicit producer receipt is distinct from promotion proof; ordinary promotion
capture still requires the current final-metadata proof. This lifecycle boundary
does not relax initialized-byte, allocation, relocation or identity checks.

## Consequences

Compiler-emitted initialized data can become canonical without a retained
binary pool, new per-function shaping, or executable credit for data bytes.
Synthetic regression coverage distinguishes external and same-module exported
calls from private calls and exercises separate data/read-only-data bases.
This establishes the source-to-module boundary needed for subsequent table
generation; it does not claim that all modules or generated tables are closed.
