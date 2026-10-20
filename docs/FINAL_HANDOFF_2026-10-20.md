# Final matching sprint handoff — 2026-10-20

No new match was obtained in the final pass. Canonical source is unchanged;
neither remaining function is ready for promotion. Resume from
`campaign/unchain`, with the current per-function shards as the detailed record.
The older summaries and controlled compiler results in those shards are history,
not the current canonical score or evidence of a stock match.

## Current state

`gmake progress` was recomputed from the current tree and linked ELF. Resolved
executable bytes remain **934,928 / 943,640 (99.08%)**. The unresolved deficit is
**8,712 bytes**, entirely in the following two `NON_MATCHING` functions.
C-only decompilation is 905,188 bytes (95.93%); the resolved total also includes
29,740 bytes of reviewed handwritten assembly. Overlays remain 100% resolved.
The function count is 1,351 matched / 1,460 total (92.53%). These measures must
not be substituted for one another.

| Function | Canonical candidate | Remaining deficit | Status |
|---|---|---|---|
| `func_800517E0` | 7,232 bytes / 1,808 words; frame `0x1B0`; 245 owned relocations | 448 relocation-masked differing words, raw 455; aligned residual 141; first mismatch `+0xDC` | `NON_MATCHING`; interpreter source unchanged |
| `func_80054B3C` | 1,480 bytes / 370 words; frame `0xD8` | 353 exact words; 17 stack-immediate differences; first mismatch `+0x148` | `NON_MATCHING`; three relocation records exact |
| `func_80011980` | 860 bytes / 215 words; frame `0xC8` | None; all 12 relocations exact | Already matched, real linked range and full ROM proved |

The collision relocation records are the three calls to `sqrtf`, at relative
offsets `0x33C`, `0x3D8` and `0x4D0`. Exact frame and relocation records do not
make its seventeen instruction differences acceptable. The old collision
70-word/frame-`0xE8` frontier and ray ten-word frontier are superseded.

Current source checkpoints are interpreter
`b932d3e014e59c0e46902262f4adb08defc4ecfc`, collision
`349847108e8ac4f873fe9a0ebaa3fb70cb70770a`, and matched ray
`4951e471b44c86bdd85d0b51b65b11f89913d734`. The two remaining functions share
`src/main/anim.c`; source ownership must still be function-scoped.

## What the final pass established

The last source experiment followed a new native compiler observation, not a
spelling sweep. The observer was built and exercised on three existing inputs:
canonical baseline, before-shift narrowing, and shared-conversion narrowing.
Six fresh stock/observer-OFF/observer-ON comparisons were whole-object
identical. Three comparisons to older frozen stock objects separately proved
the executable, data, symbol and relocation surfaces; whole-file equality
with those older objects is not claimed because input/debug paths differ.

Translation identity was authenticated independently. All three streams had
55 procedure entries; the target was joined through its actual procedure name,
entry, first source location and translation order. The observer's later
evaluation counter was not used to guess an earlier translation identity.
The parent independently checked the frozen hashes, object surfaces, identity
join and actual branch sequence.

The measured narrowing selector distinguishes these cases:

- Canonical reaches the translator with a bitwise-expression child and falls
  back to explicit conversion operations.
- Before-shift narrowing reaches it with a register child. The existing
  register-classification result rejects load folding before reference-count
  checks.
- Shared conversion reaches it with a memory-load child. Its refusal flag is
  clear and its tree reference count is one, so the translator accepts a
  signed two-byte replacement load. The original load is also two bytes:
  this is signedness selection, not a byte-width reduction.

The relevant reference count is maintained by tree duplication and release,
after local ownership changes. It is not the source use count or an optimizer
savings count. Register classification can mutate state; an observer must log
the existing result rather than invoke that helper a second time.

This measurement qualified one final source test. Authorization
`db8bc6124975f617144fc9cbdb847fa1db1f004b` pinned the current source and
`40c55751d8563b866658558ee7f1d9ae287344ae` handoff. The parent and fresh
`lane/sol-mixed-fanout` each obtained an actual zero-exit `base-only` gate.
The worker reproduced the untouched baseline before editing.

The candidate combines the raw and signed-converted halfword in the low-byte
expression, retaining the previously measured wide-carrier/layout control
and signed high-byte extraction. The extra syntactic read is disclosed;
the same nonvolatile halfword is read without an intervening write or call.
Independent semantic review covered the complete unsigned-halfword domain
under the configured signed-conversion and shift semantics.

The optimizer retains both actual owners. Native translation consequently
takes the register-child fallback, preserving the conversion operations.
The desired high-byte argument copy survives, and total temporary draws stay
at 279 GP / 147 FP. The assembler subsequently contracts the conversion pair
but leaves an identity AND and a low-byte copy in the output. Thus the
combined ownership/copy mechanism works while the no-extra-operation
requirement fails.

Untouched candidate output is **7,236 bytes**, four bytes larger than target,
with raw 875 / masked 869 differing words, unchanged frame `0x1B0`, and 245
owned relocations at nonexact placements. The comparison report's `size_bytes`
field is the 7,232-byte target extent; candidate extent was read independently
from its actual ELF symbol. The local handler grows from
canonical 62 to 63 words; target is 61. All 127 handler mappings remain
covered, with shape/naming buckets outside this handler unchanged. Bucket
agreement does not prove all immediates, branch destinations or relocation
bindings. No candidate source is adopted and no matching credit is awarded.

Parent review verified all 223 frozen payload hashes, reran the ELF/handler/
phase review, and compared actual objects for twenty fidelity receipts. Three
older context comparisons reference prior objects outside this final-cell
bundle; those were independently checked in the earlier selector review.
New native observer, retained-input and pristine assembler-debug controls pass.

The stopping reason is a measured failure of this proposal and absence of
another qualified source repair within the final packet. This is not a claim
that three identical failed attempts occurred, or that all possible source
forms have been exhausted. A future attempt must explain how the redundant
operation disappears while retaining the two owners and required copy.

## Earlier findings that the next investigator should retain

The detailed [interpreter shard](matching-triage-handoffs/func_800517E0.md)
contains the experiments and their limitations. The most useful distinctions
are these:

- The byte-extraction handler has a real structural excess, while another
  short handler is one word short. Moving instructions elsewhere to balance
  total size is not an exact repair. Two protected handler islands already
  match and must remain so.
- Before-mask and before-shift conversion forms preserve all 1,734 observed
  queue transitions, yet lose the required argument copy and are eight bytes
  short. Equal queue histories do not prove equal typed operation owners or
  target compiler history.
- Sharing only the converted owner protects the copy but folds the load and
  conversion together. GP draws fall from 279 to 277; several other handler
  shapes and names change. That candidate is twenty bytes short and worse.
- The assembler's backward copy replacement checks reads of the destination
  before recognizing the producer, including a read in the producer itself.
  Earlier forwarding can remove this protection. Inspect successive passes,
  not just the source expression or final listing.
- Required bit width and liveness are separate inputs. A native fixed-point
  demand graph explains the narrowing refusal, including conservative
  division/exception demand across the dispatcher. A graph counterfactual is
  not a source-realizable repair.
- The private floating-point cost diagnostic improves masked 448 to 443 by
  changing five bindings, with other owned bytes and relocations preserved.
  It uses inert shaping and is nonexact, so it was not adopted. Tested natural
  role replacements did not supply the required cost effect.
- The proposed OR-zero expression-copy trick for the other short handler
  lacks its prerequisite: the actual owner is already a real variable with
  a live defining store. Do not repeat that mechanism without new evidence.

The [collision shard](matching-triage-handoffs/func_80054B3C.md) records a
different, unresolved storage-origin problem. Canonical seventeen remains
best. An alternative reserve model reaches 72 differing words, but still has
69 stack differences, three floating-point naming differences, the wrong
frame and inherited volatile qualification whose semantic and adoption
obligations remain unresolved.
The controlled zero additionally requires compiler interventions and is not
a stock or promotable match.

Native collision observation now traces the address-expression reservation
through its upstream seed, propagation and final union. Copy reconstruction
sets the occurrence flag that causes pruning to clear the subtracting set;
the resulting membership selects reserved storage on later occurrences.
The earlier statement that the originating seed was unobserved is superseded.
Keeping a named pointer avoids reconstruction in an existing diagnostic but
also removes a required second address evaluation. Removing these two
reservations alone would not repair all frame/home/palette discrepancies.
A nine-float overlapping storage diagnostic realizes the proposed offsets
but fails cadence, traffic and allocation. No new collision source cell was
qualified in the final pass.

The [ray match](matching-triage-handoffs/func_80011980.md) is closed. Its
defined inert source forms have semantic disclosure and a cleanup obligation;
cleanup must preserve owned bytes, relocations, real linked identity and full
ROM identity. Do not reopen it as an unmatched target.

## Reusable workbench improvements

The corresponding `n64-decomp-workbench` changes are integrated locally on
`main`, starting with `142471506c2a2dc1e1fc2c84b763fcada3d65ca5`.
The final identity-cleanup limit is integrated in
`fbea1722791623a2b976d3b847ce7d6b617f3909`.
They extend existing laws L149/L150, the field guide, compiler instrumentation
guidance and the UGEN operand-owner guide. Packaged law and guide copies are
kept identical to their canonical documentation.

The additions cover queue-history limits, producer-inclusive copy checks,
forwarding before replacement, actual narrowing-selector order, mutable-helper
observation and translation/evaluation identity. No behavior-changing compiler
patch, production tracing profile or parser was silently introduced. Native
observers and game-derived evidence remain private. The generic findings also
belong in Mickey's [IDO learnings](ido-learnings.md).

The workbench full suite passed 3,219 tests on the initial documentation
snapshot; final prose was covered by 93 focused documentation/law/guide tests,
repeated by the parent after integration. Ruff check/format, mypy and a
Python 3.12 wheel build passed; packaged documentation bytes were checked.
Earlier Python 3.14 wheel attempts failed in the local backend environment.
Generated wheel build copies also caused a documentation-link scan failure;
preserving those generated files outside the checkout restored the same-content
checks. Neither recovery bypassed a gate or changed production code.
The final supplement passed the same 93 focused tests, independently rerun
after its integration. The full suite was not repeated for that prose-only delta.

## Integration pointers

The committed investigation trail includes width-demand provenance
`4cb51468c1a007d2308dfeb234568a2fef95ccde`, collision seed reconstruction
`27564673a78b46920e0524b79b9b222e7f00b9d2`, pre-mask closure
`34fbb92ce15a109608336901930134ab1ce01d60`, before-shift/shared-conversion
closure `f5ed605ba`, producer-read learning
`3ce231f0cda48e069b66a5b971dab1d621955b26`, and static selector/phase-identity
learning `40c55751d8563b866658558ee7f1d9ae287344ae`.
The final measured selector and rejected mixed-fanout cell are recorded in
`d6c93a33191767ff86feaa75fdf887c173464a8e`.
These are evidence/documentation commits, not additional matched functions.
The final native observation supersedes the static observer's unmeasured status.

## Preservation, validation and restart

Private evidence remains in ignored directories owned by the coordinator lane:

- `build/extension-20261020/`: width/copy/seed investigations, frozen earlier
  candidates and independent review receipts. The prepared-only observer
  status there is superseded by the exercised final-pass observer.
- `build/final-pass-20261020/cvtl-selector-frozen/` and
  `cvtl-selector-identity-supplement/`: actual observer, inputs, objects,
  hashes, compiler context, identity joins and fidelity results.
- `build/final-pass-20261020/parent-selector-review.json`: independent
  verification of 62 frozen payloads and the measured selector distinction.
- `build/final-pass-20261020/mixed-fanout-frozen-cell01/`: the rejected source,
  actual inputs, stock/observer objects, phase records, manifest, receipt and
  `review.py`. Its manifest SHA256 is
  `f546231d62c475ed26a15bae6249776c9bade0fff43462bea515db4d8af0ba18`.
  The parent's `parent-mixed-fanout-review.json` and
  `parent-mixed-fanout-fidelity.json` sit alongside that directory.
  `mixed-fanout-handoff/` preserves the worker's commit, gate and bootstrap
  recovery receipts separately.
- `build/final-pass-20261020/workbench-handoff/`: reviewed workbench patch,
  committed file hashes and validation/recovery logs.
  `workbench-mixed-handoff/` preserves the final generic supplement separately.

These are local research artifacts, not files to add to Git. Do not commit
objects, compiler sources/binaries, instruction listings, workbench ledgers,
ROMs or absolute workstation paths. Obtain an explicit frozen handoff before
reading another worker's files; committed lane objects may be inspected through
Git without touching that lane's live state.

From the coordinator lane, the frozen final-cell review is reproducible with:

```sh
.venv/bin/python build/final-pass-20261020/mixed-fanout-frozen-cell01/review.py --repo .
```

It reads frozen artifacts and prints a report; it does not compile or edit
source. Read the accompanying receipt for the original configured compile,
observer and phase-capture commands before rerunning those heavier steps.

For a restart, first inspect the current source and shards, then run:

```sh
git status --short
python3 tools/lane_status.py --base HEAD --symbol func_800517E0 --json
python3 tools/lane_status.py --base HEAD --symbol func_80054B3C --json
gmake progress
```

Consumed authorizations are not permission to repeat experiments. A genuinely
new source mechanism needs current committed source/handoff pins and an actual
zero-exit `base-only` gate in its isolated lane. For interpreter, the missing
prerequisite is a measured late elimination that removes the extra operation
without collapsing ownership, losing the copy or disturbing other handlers.
For collision, it is a source origin that preserves both address evaluations,
field cadence and owner economics while changing storage membership. Neither
is currently an authenticated source repair, and success is not guaranteed by
running longer.

Reproduce the untouched full-TU baseline before edits; compare actual configured
preprocessed context, phase formation, owned executable bytes and exact
relocation identities. An exact candidate still needs real linking, owned-range
comparison, full ROM verification and the ordinary clean-room/documentation/
scoreboard gates. Follow the progress-based stopping rule rather than opening
a blind permutation grid.

Closing `gmake check-docs cleanroom check-scoreboard` passed on the integrated
coordinator tree; normal commit hooks remain enabled. The fresh worker's first
scoreboard link lacked generated friendly resident aliases. Re-running the
normal alias generation after resident objects existed restored the committed
surface and passed the scoreboard; the failed build and recovery are preserved.
No generated alias change was committed.

The final pass changed documentation and scheduling evidence only. No new full
ROM proof is claimed for these unchanged source files. The owner's unrelated
`docs/NEXT_CAMPAIGN.md` edit and untracked `slotprobe3.B`, `slotprobe3.G` and
`slotprobe3.T` are preserved. No push or master integration was performed.
