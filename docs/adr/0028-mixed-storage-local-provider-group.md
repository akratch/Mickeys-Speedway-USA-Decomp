# 0028. One mixed-storage whole-provider LOCAL group

Status: Accepted
Date: 2026-10-24

## Context

Overlay57's result layout has six string pointers and eleven signed-halfword
column positions read by its renderer. The genuine candidate struct has C
size48, original GLOBAL ELF object size47 and a complete DATA allocation48.
These are separate facts; the existing bounded symbol-derived suffix predicate
admits one trailing zero byte without altering the original object or symbol.
The eleven-element read view does not disprove a possible original twelfth
zero element. No original-declaration claim follows from payload alone.

The six actual named bases are split across two contiguous current C inputs:
a complete readonly RO64 input and a mutable literal rawDATA48 input whose
configured CPP/source proof derives effective44. Both retain their actual
zero-sized character symbols. Existing ordered groups require writable DATA
and a fully sized root; the one-provider mixed case cannot express this group.

## Decision

Add only the explicit `--local-ro-mixed-provider-group` case. Initially select
no canonical source: the actual overlay57 source/consumer adoption needs its
own accepted proof. Existing ORT, same-owner, one-provider and ordinary ordered
DATA-group profiles retain their defaults and refusals.

- Require one complete non-executable DATA producer and its actual unique
  GLOBAL OBJECT root at zero, with no other allocated definitions. Reuse the
  accepted root/allocation predicate: either a complete sized root, or exactly
  one zero suffix byte independently derived from the actual symbol and input
  alignment. Preserve reported symbol47, allocation48 and all original bytes
  and records. The reviewed source must assert its logical C size without
  adding storage; helper metadata is not a C-type parser.
- Every original source reference must be ordered, unique, word-aligned R32,
  bounded by the actual reported root, zero-addend and name a true undefined
  GLOBAL object base. Scalar and alignment bytes survive ordinary linking
  unchanged. No retained-site identity fills a missing source relocation.
- Require exactly two distinct complete provider inputs in declared order:
  readonly RO first and mutable DATA second. Each has genuine ELF flags,
  original definitions, no code/BSS/COMMON/other allocation or relocations.
  The first has an actual GLOBAL object at zero. Section spelling establishes
  compiler storage only, not the runtime section role.
- The second input must already be an explicitly selected literal-array
  ro-member with the first input as its held readonly origin. Reuse its fresh
  compiler, actual wrapper/CPP, source/header/tool and normalization proof.
  Preserve original raw48, all original size-zero symbols and source facts
  separately from effective44. Effective bounds govern placement and target
  admission. No raw padding is silently credited as effective input.
- Ordinary GNU ld concatenates the two actual storage sections, preserving
  their complete payloads without gaps, and supplies actual named definitions
  and group offsets. The current final map independently proves both complete
  canonical source inputs are contiguous in that order and agree with the
  member's held declared runtime origin. No numeric role or target binding,
  encoded-coordinate lookup, site list or fabricated provider is admitted.
- The source-first identity map must authenticate every original reference
  before comparison to the complete owner's runtime count/type/site/order and
  identities. Reuse per-input authenticated identities; no global Resolver
  precedence change. Original raw, ordinary linker output and metadata-only
  container conversion remain separate artifacts. GNU ld may resolve pointer
  payloads; container conversion must preserve those linked payloads exactly.
- Keep the pre-link DAG source/tool/provider-only. Provider compilation and
  declared literal normalization do not depend on a final ELF. Producer link
  depends on both ordinary provider objects; final-map/current-module checks
  happen after linking. Stable source/provider dependencies enter existing
  provenance snapshots, never volatile fresh-capture temporary paths.
- Reuse loaded-byte holds, source/CPP/tool contexts, exact replay, atomic
  publication and epoch invalidation. Missing/NULL or swapped same-byte source
  records, wrong/missing/foreign/reordered providers, unsupported addends or
  extents, wrong storage/runtime role, mutations and failed warm revalidation
  refuse. A failure after success exposes neither old authority nor a partial
  inventory; restoration must permit a fresh successful retry.

## Limits

No canonical48-byte adoption, record retirement, executable credit, source-only
or native-safety claim follows from the helper. The natural producer's
const-character-pointer fields must have compatible consumer declarations.
Existing mutable-pointer/opaque font views are separate source/API debt;
qualifier removal, incompatible imports or provider qualifier changes are not
an implicit part of this helper acceptance. Any additional source reuse needs
its own reviewed module/ownership proof. Other provider classes, arbitrary
interior addends, broader groups and guessed string extents remain unsupported.
