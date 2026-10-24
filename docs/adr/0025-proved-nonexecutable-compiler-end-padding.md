# 0025. Source-proved nonexecutable compiler end padding

Status: Accepted
Date: 2026-10-24

## Context

A complete source string/scalar family and a compiler-generated readonly pool
can occupy fewer bytes than their compiler's sixteen-byte section allocation.
Their exact source object ends, relocation cells and actual load bounds establish
those ends independently of zero suffixes. Treating the whole allocation as
runtime storage would displace adjacent real inputs. Zero bytes alone do not
establish that any storage is removable.

## Decision

Add an explicit compiler-end-padding metadata profile to the existing trim and
fresh-source proof paths. Initially use it only for overlay 101's complete tail.
Preserve the original compiler ELF, bytes, symbols, records and held source/tool
context separately from every effective semantic section view.

Admit only initialized allocated non-executable relocatable sections with normal
sixteen-byte compiler alignment. The removed suffix must be zero and shorter
than sixteen bytes; its exact start must equal the independently derived end.
For a plain initialized family, actual sized object ends and the reviewed complete
source establish that boundary. For a compiler pool, every incoming actual load
and every genuine relocation cell must fit the supported bounded source access
model. Unknown widths, address forms, table bounds or targets fail closed.
No nonzero exception, instruction edit, arbitrary crop, interior removal, opaque
array, site list or synthetic witness object enters this profile.

Metadata normalization changes section extent only. Fresh captures reproduce the
configured payload and authentic definitions from the untouched raw object and
record the effective extent explicitly. Consumers use that proved span without
discarding the original records or claiming padding as runtime ownership.
Existing header/BSS completeness checks retain borrowing until their own exact
layout checks support all effective contributors.

A mutable ordinary DATA input may declare `ro-member` with one explicit ordinary
origin input. Fresh current C ownership and the unique same-module final map of
that input establish the runtime RO origin; no numeric binding or section-name
inference supplies it. Hold source, recipe, objects, origin and placement contexts
before and after derivation. Existing `data`, `ro-origin`, ORT and LOCAL profiles
and their default refusals remain unchanged.

Ordinary Splat ordering must also preserve a readonly tail whose desired start
would be displaced by the inter-section alignment. Keep its real `.rodata` input
in initialized ordering when required; leave aligned suffixes unchanged. Do not
alter global alignment or insert source padding to hide the boundary.

## Consequences

Require exact complete source payloads, all original relocation identities and
order, real function/load bounds, actual final-map geometry and complete ROM.
Test nonzero suffixes, lost cells, unsupported loads, wrong/changed origins,
stale held contexts and warm dependencies. No new proof framework, source-only
build, native-port claim, BSS ownership or executable credit follows. Further
sources need their own reviewed object/access evidence before selecting this
profile. ADRs 0021 through 0024 remain strict for their existing cases.
