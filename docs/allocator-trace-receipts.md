# Allocator trace receipts

`tools/allocator_trace_receipt.py` turns one instrumented IDO 5.3 allocator
capture into a compact, public-safe evidence receipt. It solves three recurring
problems in the trace workflow:

1. it maps a function symbol to uopt's run-local procedure ordinal from the
   exact named Ucode stream uopt consumed;
2. it refuses allocator evidence until the traced compiler output passes the
   section, relocation, and symbol fidelity gate against stock output; and
3. it records immutable baseline hashes and attempt accounting without copying
   instruction listings or raw compiler traces into documentation; and
4. it joins hash-bound workbench frame evidence, producer-emitted stack homes,
   and procedure-scoped ugen temp events into a deterministic first-mechanism
   comparison.

Raw traces and objects remain untracked workbench evidence. The receipt is a
diagnostic handoff, not a match claim.

## Capture contract

Use the same candidate source and flags for every object in one receipt:

- `candidate.o` is the output from the stock IDO toolchain.
- `candidate.B` is the retained positional Ucode input consumed by that uopt
  invocation. Create the capture toolchain with `decomp-workbench capture make
  INSTRUMENTED_IDO_ROOT CAPTURE_DIR --phase uopt --link`; the run's
  `before-<N>-*` positional input is this file.
- `index.log` is one complete instrumented-uopt capture made with `CDX_LOG=1`
  and a nonnumeric `CDX_PROC` value. The pinned profile refuses the name and
  emits one `procindex` row for every globalcolor invocation.
- `traced.o` is the object emitted by the instrumented index capture. If the
  all-procedure log is too broad for the next investigation, `detail.log` may
  come from a second compile with `CDX_LOG=1` and the numeric procedure selected
  below. Keep each capture separate; do not merge logs from several compiles.

First map the symbol before spending the detailed capture:

```sh
tools/allocator_trace_receipt.py func_80041CE4 \
  --candidate-object build/trace/particles.stock.o \
  --index-trace build/trace/particles.index.log \
  --ucode-stream build/trace/particles.candidate.B \
  --map-only
```

The command accepts the map only when all of these are true:

- the symbol owns exactly one nonempty ELF `FUNC` range and aliases agree on
  start and size;
- every retained `Uent` has a valid following procedure-name `Ucomm`;
- the named Ucode procedure count equals the complete contiguous `procindex`
  count; and
- the requested procedure name occurs exactly once.

The Ucode record order is the optimizer's procedure order; ELF order is not
used for this join. That distinction matters in Mickey because `asm-processor`
adds helper procedures which do not preserve a simple source/ELF ordinal.
Any ambiguity is an error, not a best guess. Procedure ordinals are run-local:
repeat this mapping after every translation-unit source or flag change.

Capture the reported numeric procedure and emit the full receipt:

```sh
tools/allocator_trace_receipt.py func_80041CE4 \
  --candidate-object build/trace/particles.stock.o \
  --traced-object build/trace/particles.traced.o \
  --index-trace build/trace/particles.index.log \
  --ucode-stream build/trace/particles.candidate.B \
  --uopt-trace build/trace/particles.detail.log \
  --workbench-summary build/wb/func_80041CE4.summary.json \
  --attempts 1 --budget 4
```

The full command also requires the number of detailed attempts already spent
and the task budget. It checks that the selected procedure's `p1dec`/`p2dec`
count equals the index capture before invoking `decomp-workbench fidelity`.
When the all-procedure index capture has enough detail, omit `--uopt-trace` and
the command reuses `--index-trace`, avoiding a second compiler run.
Successful text output is intentionally short. `--json` emits the complete
receipt schema for another tool.

Generate the hash-bound frame input from the same candidate object before the
trace capture:

```sh
tools/wb_compare.sh --summary-json func_80041CE4 \
  > build/wb/func_80041CE4.summary.json
```

The receipt accepts only `mickey-wb-summary-v1`, requires its requested and
candidate symbols to equal the receipt symbol, and requires its candidate
object SHA-256 to equal `--candidate-object`. A stale summary is an error. The
candidate and target frame sizes are otherwise `unavailable`; the tool does
not rediscover them from an instruction listing.

The allocator section summarizes:

- integer globalcolor decisions by phase, outcome, register histogram, and a
  stable decision digest;
- floating-point globalcolor decisions using the pinned profile's class-2
  marker; and
- unknown classes separately, so a producer change cannot be silently called
  integer allocation.

The digests make two receipts cheaply comparable while keeping web rows and
raw trace detail out of tracked files.

## Structured allocator summary

The additive `trace_summary` section keeps the original v1 receipt fields and
CLI behavior intact. Its three evidence lanes are:

- `frame`: candidate and target byte sizes from the hash-bound workbench
  summary;
- `stack_homes`: only homes for which the producer emitted a virtual or final
  offset. An explicit `width`, `bytes`, or `size` becomes `width_bytes`; an
  explicit `access` or `access_class` becomes `access_class`. Missing fields
  are `null` or `unavailable`, never inferred from a data type, opcode, or
  opaque compiler word; and
- `temp_events`: procedure-attributed ugen result rows as deterministic
  birth/pop and death/push events. Each event retains the conventional register
  name and producer source line, but omits the raw row, trace line, emitted
  ordinal, compiler addresses, and host path.

`producer_capability` audits the selected UOPT procedure independently of the
stack-home classifier. It counts only explicit `source_semantic`,
`virtual_offset`, and `final_offset` fields. A logical line, itable `sym`, or
opaque `raw10`/`raw14` word does not satisfy the audit. This makes a missing
producer surface a structured result rather than an invitation to infer a
source variable or frame slot.

Source files are reduced to a safe basename. For a multi-procedure capture,
procedure attribution is admitted only when the UGEN log contains exactly one
contiguous `DKWB-PROC BEGIN proc=N` stream whose count matches the retained
Ucode procedure list, and every free-list row carries an in-range `proc=N`.
The receipt then selects the same ordinal established by the named Ucode map.
A one-procedure input remains admissible without markers for compatibility.

The optional `--target-evidence` document permits field-by-field comparison
when compact target-side evidence has already been measured:

```json
{
  "schema": "mickey-allocator-target-evidence-v1",
  "symbol": "func_80041CE4",
  "frame_size_bytes": 32,
  "stack_homes": {
    "status": "available",
    "homes": [
      {"offset": 24, "width_bytes": 4, "access_class": "load-store"}
    ]
  },
  "temp_events": {"status": "unavailable"}
}
```

The schema is deliberately closed: extra fields, raw traces, malformed widths
or access classes, and a different symbol are rejected. Use `null` or
`{"status":"unavailable"}` when target evidence cannot establish a field.
In particular, target machine code does not itself prove a ugen temp birth;
do not transcribe a plausible event sequence from the candidate trace.

`comparison.fields` reports each lane as `equal`, `divergent`, `partial`, or
`unavailable`. `comparison.first_divergence` then names the first proved
mechanism and one bounded source lever. Its current mechanisms distinguish a
frame-size difference, stack-home count/displacement/width/access, an extra or
missing temp birth/death, and event order/source attribution. It reports
`no-divergence` only when all three lanes are available and equal. Equal known
fields plus one unavailable field remain `unavailable`, not a guessed match.

For a `animseqFreeGroup`-style trace, one additional integer pop/birth on the
path-loop line is reported as `extra-temp-birth`; the next lever is to inspect
that attributed expression for a redundant conversion, comparison carrier, or
grouping. The tool does not prescribe a source edit and the receipt remains
diagnostic evidence rather than match proof.

## Producer limits

The instrumented UGEN producer emits `DKWB-PROC BEGIN proc=N` at `f_init_regs`
and stamps integer- and FP-temporary rows with that ordinal. The receipt now
authenticates the complete marker stream against retained Ucode before using a
mixed-TU trace. In proven scope it summarizes `ALLOC_GP_RESULT` and
`ALLOC_FP_RESULT` counts, register histograms, sequence digests, and lifecycle
events. `ALLOC_GP_RESULT`/`ALLOC_FP_RESULT` are births and free-list pops;
`FREE`/`FORCE_FREE` are deaths and pushes. Request rows are validated but are
not misreported as allocated registers.

That procedure join does not create source semantics. The pinned UOPT input
contains storage classes, frame-relative offsets, itable identities, and
logical lines, but no local or parameter names. UGEN sees final code emission
and logical lines, not a surviving C identifier. Therefore the current
instrumented compiler cannot truthfully emit `source_semantic=local:NAME` for
an allocator web, and the receipt leaves it unavailable. Likewise, the stock
globalcolor profile does not emit calibrated `virtual_offset` or
`final_offset` fields. A future producer must add those fields from an
authenticated compiler structure and repeat disabled/enabled fidelity proof;
the capture tool will consume them without a schema change. Do not promote an
itable index, source line, or opaque raw word to any of these fields.

A bounded audit of the procedure-scoped producer and receipt pipeline found no
additional authentic field that can close this gap. UGEN free-list rows contain
only the procedure ordinal, physical register, emission ordinal, and logical
line. They contain no operand identity or edge endpoint, so birth/death order
cannot be promoted to a temp dependency graph. Focused UOPT captures can emit
`intf` rows between allocator-web numbers, but those are allocator
interference constraints, not UGEN data-dependency edges. Web numbers are
run-local and the producer emits no cross-stage identity joining them to UGEN
temporaries. The receipt therefore must not join either record by line,
symbol-table number, raw word, register, or relative position.

Calibration on `func_800498FC` and the independent `animseqFreeGroup` control
reached the same boundary: procedure mapping and trace-off object fidelity
passed, and procedure-scoped temp births were available, while direct source
semantics, virtual/final homes, and temp dependency edges remained unavailable.
The first function had no trace-supported source experiment; the control could
localize one extra birth to a logical line but could not authenticate the C
value that owned it. This is a producer plateau, not evidence that the missing
fields agree with the target.

The next implementable lever is a producer hook at the compiler structure that
actually owns each fact. A source-semantic or dependency record must be emitted
while a stable source/expression identity and both edge endpoints coexist, then
carry an explicit identity into the selected UOPT web or UGEN temporary. A
virtual-home hook must observe the optimizer's home assignment; a final-home
hook must observe frame-layout or spill finalization. Each record needs the
authenticated procedure ordinal and an explicit field tag, followed by
trace-disabled fidelity proof and calibration on both functions above. Until
such a record exists, extending the receipt would only re-label inference as
producer evidence and remains prohibited.

### UOPT identity and home boundary

A follow-up audit of the pinned UOPT producer found that the required composite
hook is not yet available. The productized globalcolor profile directly walks
the selected live range's interference list, so its `intf` endpoint is an
authentic allocator constraint. Its `web=N` key, however, is a phase-local bit
position, not a stable identity shared by live-range formation, both coloring
phases, copy/coalescing, and UGEN. The earlier ICHAIN table/chain fields are
also run-local IR identities and are not propagated into UGEN.

The workbench's separate `CDX_SYMTAB` patch records the owning itable entry,
but its home remains a top-of-frame `off` value. Converting that value to an SP
home requires adding a separately measured frame size; it is not a
producer-emitted final home. The current copy/coalescing reader likewise has no
productized producer: its `COPYDEC` generator remains a hash-pinned research
profile, and its documented alignment by LHS home plus assignment ordinal is a
controlled-differential hypothesis rather than stable expression identity.
Neither surface may be relabeled as the missing receipt capability.

The pinned generated UOPT source exposes these structures only through
revision-specific emulated-memory offsets. It has no typed field or propagated
identity at which this repository can add an authentic composite record without
first guessing from raw words. Consequently no new producer or receipt field
was admitted. Re-running the `func_800498FC` and `animseqFreeGroup` calibrations
would reproduce the same unavailable capability because the producer grammar,
not either input function, is the limiting surface.

The concrete next hook belongs in the workbench's source-hash-pinned UOPT
instrumentation profile. It must assign a procedure-scoped identity when the
live range is created, carry that identity through interference insertion and
copy/coalescing, and emit virtual-home and final-home records at the actual
assignment sites. If final layout occurs in UGEN, the identity must cross the
pass stream explicitly rather than be rejoined by line, register, raw field, or
ordinal. Productize that generator with its producer tests and disabled/enabled
fidelity matrix first; only then should this receipt add fail-closed parsers and
calibrate the composite records on both functions.

## Failure meanings

- A count/order failure means the symbol-to-procedure join is unproved. Do not
  substitute a remembered procedure number.
- A decision-count failure means the detail log is incomplete, stale, or from
  another compile.
- A fidelity failure means the instrumented compiler is not evidence for the
  stock compiler on that candidate.
- A budget failure means the handoff omitted or exceeded its bounded-attempt
  accounting.
- A target-evidence failure means the compact JSON was malformed, stale,
  symbol-conflicting, or tried to carry an unsupported field.
- An `unavailable` comparison is a request for narrower evidence, not evidence
  that candidate and target agree.

## Attempt-zero worker recipe

Before the first source edit, a worker should:

1. run `wb_compare.sh --summary-json` and retain its ignored JSON beside the
   workbench target object;
2. map the fresh Ucode procedure with `--map-only`;
3. capture one fidelity-clean detail trace and, only for a one-procedure input,
   one ugen trace;
4. run the full receipt with `--workbench-summary`, `--attempts 1`, and the
   assigned budget; and
5. read `comparison.first_divergence` before selecting the first source lever.

Add `--target-evidence` only for target fields already measured independently.
After every TU or flag change, remap the procedure, recapture, and regenerate
the receipt; procedure ordinals and allocator-local identities are run-local.
Preserve the JSON under ignored `build/` evidence and quote only its compact
mechanism, metrics, and next lever in a handoff.

The trace setup and the distinction between uopt globalcolor and ugen
temporary allocation are described in
[`breakthrough-campaign.md`](breakthrough-campaign.md) and the workbench's
`docs/compiler-instrumentation.md`.
