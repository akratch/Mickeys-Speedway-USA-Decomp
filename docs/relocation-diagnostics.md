# Relocation witness diagnostics

`tools/reloc_surface.py compare --explain` reports why a candidate relocation
remains unresolved and which existing proof routes were consulted. It does not
change identity resolution, tuple comparison, or `--check` acceptance.

```sh
python3 tools/reloc_surface.py compare SYMBOL \
  --candidate-object build_non_matching/src/overlays/oNNN/function.c.o \
  --target-elf build/mickey.us.elf --explain --json
```

The Python entry point is
`function_surface_comparison(..., include_diagnostics=True)`. The default
result is unchanged; opting in adds a `diagnostics` object. Reports contain
runtime relocation identities and belong under ignored `build/`, not in Git.

## Report fields

Schema version 1 has `sites` and `symbols` lists. Each site records its candidate
function-relative offset, relocation type, symbol-table index, current and
pre-redefinition names, REL addends, paired offsets, already-decided identity,
status, reason, observations, and symbol-level witness-route records. HI/LO pairing uses
symbol-table indices, so two symbols with the same spelling stay distinct.
Multiple high relocations may pair with one low relocation. An unpaired high
has no inferred addend.

`target_at_same_offset` is a positional observation. It never supplies an
identity to a moved candidate site. A shared synthetic VMA is also not a unique
identity. Resident and reserved-selector identities stay in separate namespaces.

Witness records identify their route and, when available, base identity, source,
function and module offset. `independent: true` means the existing route is
independent of this candidate's positional target correlation; it is not a new
acceptance certificate. Matched-sibling records arise only after the existing
freshness, ownership, linked-byte and runtime-tuple checks pass. The function
under proof remains excluded from that sibling search. Alias propagation keeps
`via_symbol` provenance and does not create a new identity.

Runtime HI/LO and repeated-call correlations have `independent: false`, even
where the existing resolver accepts them. Their recorded proposals cannot be
used as additional independent witnesses. Diagnostics leave that existing
resolver behavior intact.

Each site's `conflicts` separates:

- `independent_witnesses`: different base identities from independent routes.
- `alignment_correlations`: different base identities from positional routes.
- `all_proposals`: all conflicting proposals, including resolver results whose
  provenance is not independently established by this report.

Only the first category describes contradictory independent witnesses. A
conflicting alignment can result from changed instruction positions or addends;
it does not establish a contradiction between independently authenticated
objects. Symbol/call resolver disagreement is labelled as a resolver-proposal
conflict for the same reason.

Grouped symbol records collect candidate offsets, symbol-table indices,
unresolved counts, reason counts and witness routes. Human-readable `--explain`
prints unresolved groups and route findings; JSON retains every site.

## Interpreting unresolved reasons

Site reasons distinguish an unpaired HI16, an unresolved paired LO16, ambiguous
identity resolution, missing authenticated call or symbol identity, and disagreement
between symbol and call resolver proposals. Route findings give more detail:

- Canonical data ownership distinguishes a missing numeric assignment, linked
  absolute assignment, data definition, fresh unique BSS owner, or authenticated
  BSS extent. An assignment by itself is not proof of storage identity.
- Canonical call ownership distinguishes an unsuitable name from unavailable
  boundary proof. The latter is intentionally a summary: it does not claim a
  specific failed freshness or boundary check.
- Sibling absence means no eligible independent witness was returned. It does
  not assert that no semantically related sibling exists under another name.
- Runtime correlation reports incomplete HI/LO pairs, missing or duplicate
  target tuples, disagreeing pair identities/addends, mixed relocation types,
  definitions requiring owner proof, invalid extents, insufficient repeated call
  sites, out-of-overlay calls, and conflicting base proposals.

A route finding such as `aligned-pair-proposal` describes an observation, not
necessarily a rejection or successful resolution. Inspect the site's final
status and the other findings. Some proof failures still raise the same
comparison error as before; `--explain` does not turn rejected input into a
partial successful comparison.

The next action is to obtain the missing independent evidence: authenticate a
canonical definition and its owner, or prove an exact sibling's relocation and
then establish the candidate's symbol binding. Do not rename or merge aliases
merely because target tuples align. After any binding change, repeat ordinary
object, relocation, linked-range and ROM proof.

## Explicit cross-overlay reserved-storage witnesses

`compare --reserved-storage-witnesses build/witness-bindings.json` optionally
imports a named external from an independently exact canonical function in
another overlay. The JSON has `schema_version: 1` and a `bindings` list; each
binding contains only `symbol`, `source_overlay`, and `source_function`.
The candidate must actually reference that same undefined external name.
No arbitrary alias mapping, supplied raw object, numeric-address match, or
candidate target-site correlation supplies the imported identity.

Each named witness triggers a fresh configured full-TU compile under the
existing compiler/provenance machinery. Source, dependencies, compiler tools,
flags, recipe, metadata tools, source directory, and canonical target inputs
are hashed before and after capture and replay. Only symbol renames and
zero-tail trimming are admitted. The replay must reproduce every allocated section's geometry and contents
(without reading NOBITS as file data), all relocation identities across every
section, and non-debug symbol definitions normalized by section name. Compiler
FILE records and debug-only metadata are excluded. All owned compiler
instruction bits, linked ROM bytes, and the complete runtime relocation shape
must agree.
Capture receipts and raw objects remain under ignored
`build/reserved-storage-witnesses/`.

Only reserved selectors are importable, and each selector keeps its own
namespace. Overlay-local identities, conflicting witnesses, stale input,
nonexact or moved owners, and actual same-name local definitions fail closed.
For an explicitly proved foreign name only, the same-overlay numeric
whole-BSS fallback and positional correlation cannot override its namespace.
Independent resident/name and exact-sibling conflicts still fail closed.
Inputs without explicit bindings retain their existing comparison verdicts.

The JSON comparison result includes the binding and complete capture receipt.
This standalone proof does not yet feed promotion/preflight automatically;
that bridge remains required before promoting any candidate that relies on
an imported witness. An improved resolved-identity count is not matching credit.
