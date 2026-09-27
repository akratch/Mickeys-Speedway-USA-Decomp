# Independent storage-use evidence

`tools/storage_view.py` reproduces an already exact overlay function and proves
the runtime storage identity of an external that its source actually uses. It
accepts no candidate alias, target-site offset, supplied object, or manual
identity. Reports and compiler captures stay under ignored `build/storage-views/`.
This is diagnostic evidence, not a new relocation resolver or promotion route.

```sh
python3 tools/storage_view.py \
  --source-function func_overlay_060_F0000334_18BA10C \
  --external D_8007C0E8_o060Reloc \
  --resident-owner D_8007C0E8 > build/storage-views/course.json
```

The source function must already have complete canonical preflight evidence and
exact linked ROM bytes. A fresh configured full-TU compilation is captured with
source, includes, assembly dependencies, compiler, recipe, metadata tools and
target inputs bound before and after the operation. The accepted metadata recipe
is replayed, and every allocated section, relocation and non-debug symbol must
reproduce the configured object. Metadata must preserve the compiler's instruction
bits and may discard only zero text alignment. The requested external's records
and instruction fields must be unchanged even where other sites undergo reviewed
metadata processing. Filters and section externalization are outside this tool's
current scope.

The existing exact-sibling proof supplies the external's identity from that
independent owner's runtime table. The function name need not equal its source
filename. Local overlay identities remain local; reserved selectors remain
reserved. Text addresses cannot be reported as local storage. The report's
`independent-storage-use-proved` status proves the address used by an actual named
external, not the size or effective type of an inferred subobject.

With `--resident-owner`, the tool additionally requires a sized, initialized,
named resident object. Its input-object definition, link-map placement, linked
bytes and retail bytes must agree. The view must start at that object, rather
than merely fall somewhere inside a containing allocation. BSS and data with
input relocations are not accepted by this additional route.

The DATA1/DATA2 transformation is a reviewed rule from `ResolveRelocAddress`,
whose canonical function is independently compiled and checked again. Its
preprocessed function body must reproduce the reviewed digest in the tool.
For a loaded resident module at its canonical base, both selectors add their
export offset to the named resident data anchor. Equal physical addresses do
not merge those two runtime namespaces. A changed loader body requires a new
semantic review; updating a digest alone is not evidence.

A deliberately small instruction-use recognizer can report a straight-line
indexed signed/unsigned halfword load and a direct call's delay-slot argument.
It stops at unsupported operations, other relocations, and control flow. It is
not a complete footprint or a callee-effects analyzer. Empty observations mean
no supported pattern was found. Observed width, signedness and scale do not
prove index bounds, all accesses, a complete C type, or absence of writes.

## Course-view result and remaining distinctions

The exact overlay 60 prefix supplies an independent Course IDs witness through
its real external, even though its reviewed recipe also relocates a separate
switch table. Fresh replay authenticates that recipe without importing the
switch table's identity as storage evidence. The resident table owns 48 bytes;
the observed access is a signed halfword indexed at a two-byte scale and passed
to the blur-query call. The exact resident consumer separately describes six
groups of four signed halfwords. The observed access does not prove the index
range of a different caller.

The same report can omit `--resident-owner` to establish an existing named
same-overlay use. In overlay 57, the exact interface updater supplies the
selection name, and the exact mode-input handler supplies the output-pointer
and character-table names. Their identity witnesses are useful independently
of whether every table bound or record field has been reconstructed. A new
candidate must actually use a compatible authenticated view; this report does
not bind its old friendly aliases automatically.

For an owned-base-plus-offset reconstruction, a future extension must prove
that offset through an independent producer/consumer, then state the supported
extent and access contract separately. Containment alone is not an alias proof.
The current tool intentionally does not invent such views. Promotion/preflight
still needs an explicit, fresh receipt bridge before any candidate relies on
this report for final acceptance.
