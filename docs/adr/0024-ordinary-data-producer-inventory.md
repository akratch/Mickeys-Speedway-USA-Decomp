# 0024. Authenticate complete ordinary data inputs in producer inventory

Status: Accepted
Date: 2026-10-24

## Context

Overlay 3 has two genuine complete ordinary C initialized-data inputs beside
its main translation unit. They need no module linker or relocation imports.
The producer inventory currently discovers text owners and selected linked
pointer inputs. Omitting ordinary inputs prevents the unchanged complete-body
BSS check from authenticating this module even after its actual BSS is in C.

## Decision

Add an explicit proof-only Make selection for complete ordinary writable C
`.data` inputs. Initially select overlay 3's threshold and search-data inputs.
The marker changes no compiler, postprocess or linker command. Every marked
target must have one canonical projected source input and pass authentication;
missing, ambiguous or simultaneously module-linked selections fail closed.
Do not auto-admit other canonical data or special-case a module number.

Reuse the existing ordinary-provider capture. Fresh untouched compilation must
reproduce all allocated bytes and section descriptors, symbols and ordered
relocations of both the ordinary raw and configured objects. Require pure
writable initialized data, genuine unique GLOBAL object definitions, no text,
RO, BSS, COMMON, assembly fallback, inline assembly, imports or relocations,
and one complete nonoverlapping final-map contribution with exact linked bytes.
Authenticate each named definition against that same raw source and final input.

Each selected input also declares exactly one held runtime role: `data` or
`ro-origin`. Compiler section spelling and mutability do not establish this
partition. For same-module LOCAL references to these genuine source definitions,
DATA uses the actual module text end, while RO uses the authenticated declared
input's actual map start. Reject missing/conflicting roles and duplicate RO
origins. Recheck the expanded role map with the other held contexts; record the
role and derived origin in the bound provider receipt. Cross-module, ORT and
unselected paths retain their existing rules. The module's global RO-base
borrowing remains explicit; this does not silently retire it. Independent table
and ROM proofs must reject an incorrect declaration rather than repair its
role from a retail relocation site.
An IDO zero-sized character-array symbol establishes its base only; never infer
its extent or the input's allocation from that size. Complete fresh section
reproduction establishes input ownership, including ordinary compiler alignment.

Publish these objects only inside the existing atomic data-producer guard,
after all source, dependency, recipe, configured/raw, actual ELF/map/link,
compiler and tool holds succeed. Failed or recursive revalidation cannot expose
partial or old inventories. Preserve the existing namespace invalidation and
warm metadata dependency closure. No cached certificate substitutes for the
current fresh compilation and held context.

Keep `header_bss` unchanged: it must independently authenticate every actual
initialized and BSS contributor, exact ordinary alignment and complete extents.
Admission alone cannot establish a module's BSS or retire its borrowed header.
ADRs 0021, 0022 and 0023, promotion proof and unsupported-case refusals remain
unchanged. These inputs do not run through a module linker.

## Consequences

Real ordinary C definitions can enter source-based identity and header checks
without pretending they are pointer tables or complete single-TU modules. This
adds no executable or initialized-data ownership credit. Multi-input ordinary
module linking, native direct bindings and original declaration/index safety
remain separate obligations. Further complete plain inputs require explicit
selection and the same generic proof, not a broader automatic registry.
