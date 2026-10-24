# 0022. Bounded cross-input LOCAL data references

Status: Accepted
Date: 2026-10-24

## Context

ADR 0021 deliberately excludes general cross-input LOCAL imports. A complete
pointer input can nevertheless have a genuine ordinary C provider: overlay 8's
surface-name table addresses independent mutable character arrays in one
already-owned input. IDO emits real global object definitions for those arrays
with zero ELF sizes. Their runtime RO origin is distinct from the compiler's
writable `.data` section. Neither section spelling nor a generated name proves
that loader role.

A stock compiler and ordinary GNU linker reproduce the complete pointer input.
The raw undefined operands are zero; the linker supplies actual provider-relative
values. The old raw/configured equality rule therefore cannot prove this case,
and its stricter ORT and same-owner LOCAL cases remain necessary.

## Decision

Admit this additional case only through the existing data-only module registry:

- One complete writable pointer input, with no executable, rodata, BSS, COMMON
  or assembly fallback, consists entirely of ordered, aligned, bounded R32 sites.
  Every source operand is an undefined named import with zero addend.
- One explicit `--local-ro-origin-object` recipe argument names the complete
  ordinary provider input. Its start declares the runtime RO origin independently
  of C constness and ELF section spelling. It lists no sites, numeric bindings
  or target-name exceptions. Mutability is preserved.
- Every import resolves to a unique actual global object base strictly inside
  that provider's initialized section. A zero ELF size permits only its named
  base with zero addend: it establishes no object extent or interior address.
  No size patch, ABS alias, encoded-coordinate interpretation or generic
  unresolved-symbol fallback supplies an identity.
- Untouched compiler inputs and their relocation records are preserved.
  Ordinary GNU ld applies bindings derived from real provider definitions.
  Generated import bindings remain private namespaced metadata, not C owners.
  Container conversion changes ELF headers only.
- Fresh source, recipe, tool, provider and loaded-object observations are held
  before derivation and checked afterwards. The actual ordinary linker command
  and products are reproduced. Actual canonical rows, unique final map inputs,
  complete payloads and definitions authenticate placement and full identities
  before retail records are compared.
- Exact R32 count, type, site, order, runtime role/base and full target identities
  are required. The new input's linked payload is proved from ordinary linker
  reproduction and its derived relocation values; old ORT/same-owner payload
  rules remain strict. Replacing a zero-offset pointer with NULL must fail even
  when the configured word remains identical.

The provider object is a normal Make prerequisite. Its definitions and the
whole-input origin declaration suffice for pre-link bindings; the future final
ELF/map must not become a prerequisite of an object needed to build that ELF.
Post-link proof separately authenticates the declared relationship. Stable
ordinary object paths, source dependencies and role arguments enter existing
capture snapshots; temporary capture paths must not become build dependencies.
Failed revalidation exposes no earlier admitted inventory or cached authority.

## Consequences

The approved guard is one same-module, one-provider, zero-addend named-base
case. General grouped LOCAL references, mixed providers, cross-module non-ORT
imports, executable targets, arbitrary interiors and unsized allocation claims
remain unsupported. Function promotion retains its default non-ORT refusal;
only the bounded data guard supplies an authenticated local identity map.

Existing source/compiler/context machinery and focused synthetic tests are
extended, without a new normalization or proof framework. Required validation
includes actual ordinary linking, missing/ambiguous/wrong-role/addend and held
mutation refusals, warm dependency behavior, all unaffected runtime identities,
zero retail differences, the expected full ROM and normal gates.

Only freshly proved residual removals may ratchet down. Unrelated layout and
RO-base borrowing remains explicit. This decision grants no executable credit,
source-only build, general relayout, universal C-safety or native-port claim.
ADR 0021 remains unchanged for all its existing cases and exclusions.
