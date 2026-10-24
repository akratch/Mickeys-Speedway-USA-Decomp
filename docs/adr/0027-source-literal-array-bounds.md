# 0027. Source-authenticated bounds for complete literal-array inputs

Status: Accepted
Date: 2026-10-24

## Context

IDO can emit real named char-array definitions with ELF object size zero.
This does not make the arrays unbounded: a complete literal initializer defines
their C extents, including its implicit terminator. The current compiler-end
profile correctly requires sized objects or independently proved pool reads and
must continue refusing unsized inputs by default.

A genuine five-array O57 family emits48 allocated DATA bytes. Its restricted
source declarations and actual CPP independently establish C extents3,3,3,12,20;
actual raw bases0,4,8,12,24 derive end44. Same-input allocation-free sizeof
controls corroborate these facts without changing actual size-zero symbols or
adding allocated witness values. Raw48 first differs from the retained image
at46, illustrating why payload zero/mismatch scanning is not end authority.

## Decision

Add the explicit `--literal-array-end SOURCE` complete literal-array end profile in the existing
provenance/metadata path. Preserve all prior profiles/default refusals.

- Accept a closed complete declaration class of named GLOBAL plain char[] or
  const char[] arrays initialized by one direct unescaped ordinary ASCII
  string literal each. Initial class rejects macros/conditionals in the owner,
  explicit bounds, pointer/aggregate initializers, concatenation, escapes,
  non-ASCII/wide/UTF encoding, expressions, attributes, other storage or code.
  Leading literal includes are held and independently preprocessed; effective
  C may add only nonallocating typedef context, with exactly the same arrays.
- Reuse the existing configured compiler/preprocessor, existing vendored C
  parser and source/context machinery. For the configured asm-processor wrapper,
  replay its actual frontend transformation and compiler include order: the
  otherwise empty temporary input directory, declared include directories, then
  the original source directory. Discover and hold the chosen header closure
  from that transformed input before compilation/proof snapshots. Bare IDO
  retains its original source-directory lookup. Hold the frontend/interpreter
  sources as well as the existing parser/tool closure. Bind actual loaded source/CPP/raw bytes
  and parser/tool/header/recipe dependencies before and after parsing. The
  profile owns the vendored parser's first loading: an already-loaded namespace
  without its profile loading identity refuses. The existing lexical helper's
  loaded-identity guard must also pass. Publish
  no proof that describes later reread bytes instead of the loaded observations.
- Derive C length including NUL from the supported literal syntax. Authenticate
  actual unique raw GLOBAL STT_OBJECT definitions, types/sections/order/bases,
  full literal bytes and the entire definition set. Retain their original ELF
  size zero. No guessed size or symbol/header patch creates extent authority.
- Restrict to one complete ordinary initialized nonexec DATA or RO input with
  correct mutability flags, no other allocations/COMMON/relocations/assembly.
  Compute final meaningful end from the authenticated declarations and raw
  bases; then apply the existing bounded zero end-alignment predicate. The
  numeric recipe size may assert equality but never supply the bound.
- Retain original raw bytes/hash/records separately from an authenticated
  effective section-size view. Only permitted metadata normalization changes;
  no instruction/nonzero byte or relocation deletion. Default compiler_end
  still refuses unsized objects, and no fabricated sized witness is produced.
- Full source adoption additionally needs fresh source/helper reproduction,
  independent unique current map/wholepayload/runtime proof and truthful role
  declaration. Ordinary compiler section spelling does not choose runtimeLOCAL
  role. An initially literal-array `ro-member` may explicitly declare
  `PLAIN_DATA_RO_ORIGIN_SECTION = .rodata` with its one
  `PLAIN_DATA_RO_ORIGIN_OBJECT`. This admits only a fresh complete projected
  readonly C origin with genuine GLOBAL object base zero, unchanged original
  symbols, unique same-module map, and origin end exactly at member start.
  Authenticate every real definition through the existing full provider proof;
  hold its source/recipe/raw/configured/map context through atomic publication
  and warm revalidation. Missing or stale origin/role refuses. The default DATA
  origin and all other profiles remain strict. This helper permits no O57
  source adoption by itself; that requires the updated integrated source state.

## Limits

The helper/profile adds no source adoption, record retirement, semantic deadness,
original-name/type recovery, constness, BSS/header capacity, source-only or
native-safety claim. New literal syntax or producer shape needs its own
review and negative controls; unsupported cases refuse.
