# 0026. One mixed aggregate with an actual read-only LOCAL provider

Status: Accepted
Date: 2026-10-24

## Context

The overlay58 rank-text family has six string pointers and eleven glyph
positions read by the renderer. Its genuine C struct allocates48 bytes,
including natural alignment. An allocation-free compiler assertion proves its
C size. Stock IDO and the normal wrapper both report the unchanged global ELF
object size47 in a48-byte DATA input. These are distinct facts; this decision
does not rename that symbol extent48 or diagnose a compiler bug. The old
header's possible twelfth glyph slot remains separate view/capacity debt.

The six pointers name actual const arrays in one already-owned complete
read-only RO input. Prior LOCAL profiles require writable providers and either
all-pointer inputs or the separately accepted ordered multi-input aggregate.

## Decision

Add an explicit `--local-data-aggregate` case with exactly one declared
`--local-ro-origin-object`. Existing ORT, same-owner, pure one-provider and
ordered-group defaults retain their guards and refusals. Initially select only
the independently reviewed overlay58 rank-text source.

- Require one actual GLOBAL OBJECT root at DATA zero, with no other allocated
  definitions. All original ordered, aligned, zero-addend undefined R32
  references must fit inside its actual reported symbol span. Preserve every
  original raw byte and relocation; no synthetic witness encodes a site list.
- Authenticate the full raw/configured input, source and recipe, unique current
  map contribution and unchanged actual final root size. A complete sized root
  is supported directly. The additional reported-symbol/input difference is
  restricted to one zero byte, admitted by the existing compiler-derived
  sized-object/end-alignment rule. The rule derives its end from actual symbol
  bounds. It neither crops this input nor drops its relocations. The original
  compiler input and full ordinary linker payload stay separate and intact.
- The shared sized-object/suffix predicate alone does not authenticate outgoing
  references. This profile validates every original R32 within the root first;
  the default compiler-end trim path still rejects outgoing DATA relocations.
- Require the genuine source's allocation-free C-size assertion and reviewed
  field layout. Receipt fields separately disclose reported root extent,
  compiler object end, complete input allocation and reviewed logical C extent.
  The latter is the reviewed whole-input source contract, not a C parser or
  an inference from the ELF symbol size.
- A provider has exactly one complete ordinary initialized writable DATA or
  read-only RO section, with genuine flags, types and allocation. Reject text,
  other meaningful sections, COMMON, executable/wrong flags and relocations.
  Fresh source/compiler/provider/map proofs authenticate actual unique named
  global object bases. A size-zero char-array symbol proves a base, never an
  invented string extent.
- Runtime RO origin is the authenticated complete provider input start,
  independently of compiler section spelling. Ordinary GNU ld applies actual
  bindings and reproduces every scalar, pointer and alignment byte. Metadata
  container conversion preserves linked payload; original raw records remain
  independently authenticated. Derive all identities before comparing the
  complete runtime-owner record set and order. No retained-site identity fill,
  coordinate decoding, numeric role binding or resolver fallback is introduced.
- Retain all current source/provider/recipe/map holds, atomic publication,
  lifecycle invalidation and warm revalidation. Missing-zero relocations,
  same-byte target mutations, wrong providers/roles, drift and unsupported
  tails refuse. New reuse needs its own accepted source/module proof.

## Limits

This adds no executable credit, unknown BSS ownership, header closure, native
safety, general section admission, arbitrary aggregate tail acceptance or
source-only/zero-oracle claim. The existing main/header views remain unchanged.
