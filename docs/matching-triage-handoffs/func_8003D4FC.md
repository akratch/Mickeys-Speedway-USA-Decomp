<!-- plateau-handoff:func_8003D4FC:start -->
### `func_8003D4FC` plateau handoff

- source: `src/main/particles.c`
- score: 0 differing words
- frame: 0x138
- relocations: 14
- first mismatch: none
- summary: MATCHED. This header claimed an open residual until 2026-09-16, when tools/check_shard_metrics.py reconciled every shard against the ranking; the function had already left the queue. The claim it carried was: 995/1068 words; 14 tuples exact. Five flat controls after faithful sweep. Next: complete UGEN queue-transition and emitted-row evidence.

The September 8 register-residual audit used assignment base
`edecdb8232aa7728d82d385b0e81ec00f3cb916e` and retained the reconstruction
from `969adde71f114d7d94cd95d89064e4a548248135`. The configured full-TU
baseline and final remeasurement both have 995/1068 positional words exact,
73 raw and 73 relocation-masked differences, 4272 owned bytes,
frame `0x138`, and all 14 relocation offset/type/identity tuples exact.
The owned ROM range remains `0x3E0FC` through `0x3F1AC`; no padding or new
matched bytes are credited. The candidate body is unchanged and guarded.

Workbench `diagnose` reports `register-permutation`. The current reader
routes overlapping integer-register roles to `register-role-audit`; the
assigned `forced-color-oracle` levers 17–19 were also read. The copy/coalescing
lever gates did not fire. The fresh skeleton scan again selected the existing
JFG placeholder sibling; no new donor material or names were adopted.

The named Ucode stream maps the function uniquely to procedure 7 among 44
procedures. Instrumented UOPT and UGEN builds each pass stock-output fidelity
for text, rodata, data, symbols, and relocations. The selected UOPT capture has
85 phase-one decisions and no phase-two decisions. No successful coloring
decision assigns either exchanged register; the UGEN capture directly returns
both as temporaries, among 235 GP allocation results. This is candidate-role
evidence, not a reconstructed target allocator trace.

`oracle plan` refuses the repeated phase-one web 494 produced during splitting
inside this single invocation. No force grid or forced-color reachability
proof was claimed. FIFO replay also remains unproved: the available producer
records free requests without successful queue-appending transitions, and no
calibrated emitted-index/object-row map is present. Do not turn this incomplete
trace into a target queue or a source-variable identity.

Attempt 1 was one 20-minute bounded stock-compiler permutation batch. The
preserved-macro importer initially failed actual-input context self-comparison;
that preparation failure is not an attempt. A lane-local empty macro-preserve
configuration then passed actual emitted/full-TU instruction fidelity, all
runtime relocation identities, and the unchanged context self-comparison.
The batch kept every normal readiness and preservation gate active.

Its weighted scratch score moved from 370 to 180 after approximately
54 seconds and never improved further. The workbench classifies the measured
window as `P_STUCK_FLAT`; the adapter records the project runner's field mapping
and the retained files' timestamps. The winning mutation adds a synthetic
constant `if (1)` around the first cached material call. Its retained stock
object has 3 raw and 3 masked differing words, first `+0x238`,
exact size/frame, and all 14 relocation tuples exact. It removes the register
exchange but leaves a scheduling residual. It is a nonexact source-shaping
diagnostic, remains ignored, and was not adopted or pursued under a different
playbook. Another saved candidate additionally truncates the vertex DMA
address to sixteen bits and was rejected for changing behavior.

The nine scoped source checks retained all recovered types and call identities:

- Attempt 2, `de_morgan_material_guard`: 73 raw/73 masked differences, 1068 words, frame `0x138`, first `+0x2D0`, 14 candidate relocations and 14 exact tuples.
- Attempt 3, `conditional_material_guard`: 978 raw/978 masked differences, 1073 words, frame `0x140`, first `+0x0`, 14 candidate relocations and 1 exact tuple.
- Attempt 4, `split_material_guard`: 1072 raw/1072 masked differences, 1082 words, frame `0x140`, first `+0x0`, 15 candidate relocations and 0 exact tuples.
- Attempt 5, `commute_material_comparands`: 76 raw/76 masked differences, 1068 words, frame `0x138`, first `+0x208`, 14 candidate relocations and 14 exact tuples.
- Attempt 6, `goto_material_join`: 73 raw/73 masked differences, 1068 words, frame `0x138`, first `+0x2D0`, 14 candidate relocations and 14 exact tuples.
- Attempt 7, `lexical_material_scope`: 73 raw/73 masked differences, 1068 words, frame `0x138`, first `+0x2D0`, 14 candidate relocations and 14 exact tuples.
- Attempt 8, `join_material_header_call`: 73 raw/73 masked differences, 1068 words, frame `0x138`, first `+0x2D0`, 14 candidate relocations and 14 exact tuples.
- Attempt 9, `join_material_block`: 73 raw/73 masked differences, 1068 words, frame `0x138`, first `+0x2D0`, 14 candidate relocations and 14 exact tuples.
- Attempt 10, `separate_material_argument_lines`: 73 raw/73 masked differences, 1068 words, frame `0x138`, first `+0x2D0`, 14 candidate relocations and 14 exact tuples.

Stopping evidence: attempts 6–10 form five consecutive controls with no new
residual, source-web identity, or usable causal distinction. An independent
owned-function comparison shows that every one emits exactly the baseline's
instruction words and relocation fields. The explicit join, plain lexical
scope, header/call line join, whole-block line join, and separate argument
lines therefore collapse to the same object basin. This is a measured stall,
not an attempt-count ceiling. The best admissible body was restored before
finalization; rejected and regressing sources and objects remain retained.

The concrete next lever requires fresh authorization and better producer
evidence: capture successful UGEN queue transitions and calibrate emitted
indices to object rows on paired baseline/diagnostic builds. Identify the first
changed temporary event and its actual expression before replacing the
synthetic condition with an evidence-backed source operation. Do not repeat
these predicate/scope/line spellings or the same flat search configuration.

Evidence lives under ignored `build/wb/t3-1/` and the function's ignored
permuter run directory. The durable sweep receipt and artifact bundle are
recorded in `build/wb/t3-1/completed-search.json`. No instruction rows, traces,
objects, or generated scratch source are part of this commit.

Commands included `tools/wb_compare.sh --summary-json func_8003D4FC`, workbench
`diagnose`, both routed guides, `capture make`, `fidelity`, `trace-globalcolor`,
`trace fifo`, `oracle plan`, `permute-doctor`, the project bounded batch runner,
`permute classify`, and `tools/finalize_plateau.py`.

The final `gmake verify` passes with the expected US ROM SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`. This validates the canonical
assembly fallback and matched neighbors; it does not promote the guarded C.

<!-- plateau-handoff:func_8003D4FC:end -->
