# Final resident functions: research pass, 2026-10-17

Research base: `2e7e8f1c489f1381b2e7138f6cf225df84167473` on
`campaign/unchain`. No game source changed, no new match, no promotion or push.
This is a research handoff, not an assignment or a matching acceptance receipt.

## Reconciled state

Fresh `tools/progress.py` output: 934,068 / 943,640 executable bytes resolved
(98.99%), including 468,592 / 468,592 overlay bytes. The remaining guarded C
owns 9,572 bytes. The separately reported 29,740 verified handwritten-assembly
bytes are not C decompilation credit. Overlay 64 is integrated.

Configured full-TU stock compiles reproduced the current ranking:

| Function | Owned bytes | Masked differing words | Aligned residual | Size delta |
|---|---:|---:|---|---:|
| `func_80011980` | 860 | 10 | 205 exact, 10 naming | 0 |
| `func_80054B3C` | 1,480 | 70 | 300 exact, 70 immediate-only | 0 |
| `func_800517E0` | 7,232 | 596 | 1,559 exact, 115 naming, 95 immediate, 51 different; residual 261 | 0 |

These are candidate-object comparisons, not ROM-exact C. The interpreter's
alignment also has 12 candidate-only and 12 target-only words. Equal total
size does not mean all handler boundaries agree. Its freshly measured
per-handler shape-plus-naming sum is 158; historical 161/181 totals are not
the current baseline. That metric erases immediates and is not an exactness test.
First masked mismatches are respectively `+0xC0`, `+0x0` and `+0xDC`.

The assignment gate reports all three targets `active`. I inspected committed
Git objects, not sibling working directories. Grok's latest two handoffs are
on `lane/track-11980` at `74325a8d20e9b06e962c629bd3d0268224ee2144` and
`lane/anim-close` at `ebeddd39a7241b125ff32ed9a033c7623c1b5bcf`.
The canonical bodies still reproduce 10 and 70. `docs/NEXT_CAMPAIGN.md`
contains an older campaign snapshot and pre-existing local edits; it was
preserved. Broadly merging either older lane would also pull unrelated
historical differences; review target-scoped handoffs before integration.

## New result: the collision spill graph really requires five slots

The important distinction is between a greedy allocator using five slots and
a graph that cannot fit in four. I measured the latter on the unchanged
`func_80054B3C` candidate.

A private, read-only IDO 5.3 observer records the two expression IDs at the
actual `spilltemps` region-co-membership test. This is the slot-conflict
relation, not the register allocator's live-block relation. The observer
changes no compiler state or decisions. Its pristine generated source has
the workbench's accepted IDO 5.3 hash. Both observer-OFF and observer-ON
full-TU objects are whole-file identical to stock; text, data, rodata,
relocations and symbols all pass fidelity. OFF emits no observer records.
The retained named Ucode input independently maps the function to procedure
37 out of 55, rather than guessing from ELF order.

There are nine spill owners and 17 conflict edges. Exhaustive graph coloring
finds a minimum of five slots. More decisively, the graph contains these two
five-member cliques, each requiring five distinct slots:

| Common four owners | Fifth owner |
|---|---|
| `4a`, `b*b`, `-b`, `2a` | fraction expression |
| `4a`, `b*b`, `-b`, `2a` | later discriminant expression |

The fresh expression report identifies those common owners as 220, 224, 233
and 235 in this run; fraction is 236 and later discriminant 243. These IDs are
run-local. The graph independently predicts all nine observed first-fit spill
assignments and all 20 candidate-slot conflict checks.

Consequences:

- **Renumbering this unchanged graph cannot reduce five slots to four.**
  Overlay 64's successful numbering lever does not transfer by itself.
- Removing either position-pointer owner, the first discriminant owner,
  fraction alone, or later discriminant alone still leaves a five-slot graph.
- Removing any one of the four common owners gives a four-slot graph.
  An abstract deletion of any one of the six edges between those four owners
  also permits four slots. This is a mathematical diagnostic, not a claim
  that a legal C rewrite can delete that edge without changing other edges.
- Grok's shared-square direction is plausible, but it is **not uniquely
  required**. A representation change to `4a`, `-b`, or `2a` is another
  possible way to change the same obstruction. Plain named assignments,
  commuted multiplies and source statement order have already failed to do so.

This graph result concerns allocated slots. Allocated reserve, emitted local
high-water and final frame size remain distinct. A four-slot graph alone
does not prove the target frame or offsets.

### Why frame size alone is an inadequate objective

The 70 immediate differences comprise 68 SP-relative memory accesses and two
frame adjustments. The initial save area and low scratch homes already agree.
Most declared homes and incoming argument accesses are displaced by 16 bytes,
but the quadratic saves need a different mapping:

| Candidate SP offset | Target SP offset | Use phase |
|---:|---:|---|
| `0x50` | `0x58` | coordinate calculations |
| `0x50` | `0x94` | quadratic calculation |
| `0x4C` | `0x88` | quadratic calculation |
| `0x54` | `0x8C` | quadratic calculation |
| `0x58` | `0x90` | quadratic calculation |

One candidate slot maps to two different target slots across phases.
Consequently neither a uniform frame shrink nor a single offset renaming
solves this candidate. This is observed access geometry; the target alone
does not label a slot as an original C variable.

Grok's deletion of `ey`/`ez` reaches frame `0xE0` while retaining all 70
differences. The negative-square spelling reaches `0xD8` but adds 12 bytes
and changes instruction shape. Both are useful controls, neither is a match.
The earlier kept-coordinate-home experiment reached the target frame and
offsets under diagnostic forces, but altered UGEN's scratch-register sequence
because a symbol reload and an expression-temporary reload are ordered
differently. Those are two separate baselines worth retaining, not competing
scores from which to discard the higher one.

**Next collision packet:** work against the exact slot-access map and both
five-way conflicts. Start by tracing which code-motion region creates a chosen
core conflict; then test one representation/lifetime change predicted to
remove it. Measure slot graph, emitted local area, frame, every stack access,
and UGEN pass/reload order together. A same-instruction result with a changed
graph is informative even if the scalar score stays 70. Conversely, a smaller
frame with unchanged wrong home relationships is insufficient. If the
kept-home branch is used, its immediate goal is the specific reload-order
and two-pass register-ring discrepancy, not another global coloring sweep.

## Ray query: two coupled allocation decisions, not one missing color

The current body is 215 words long, with ten mismatches; it is not a ten-word
function. Five early face-address uses and five later edge-loop uses are
coupled. Follow-up full-TU reproduction distinguishes the controls: the
canonical two-force control retains one operand-order difference, while the
sibling byte-offset control reaches zero masked differences under its two
forces. Neither is a stock match. Forcing only the early address exchanges
one five-word error for the other.

The more informative source control is the matched sibling's first-edge-read
shape in `func_80010654`. In the recorded experiment it creates the early
zero-emission edge reservation and a zero-score forced allocation. It also
exposes the phase boundary: edge/face have 22 interferers and enter phase one,
whereas the desired phase-two partition needs 21 while preserving the
index/cursor/inside assignments. The natural memory-read keep produces an
extra load web, which restores the very interference it was meant to remove.
This is why repeated rereads and simple local renames keep returning to the
same basin.

Grok's ladder establishes that additional webs move the face through available
colors. It does not establish a route that preserves the remaining assignment
or instruction stream. A ladder reaching the desired face register while
increasing the total naming residual is a mechanism control, not progress
toward an admissible candidate.

There is also a concrete validity issue in the recorded `sign == sign`
outer-face probe: canonical `sign` has no initialization before the first
edge loop. Reading it before that loop is not a defined inert diagnostic.
Do not reuse that probe as evidence of a legal source route. Similarly, any
signed doubling of encoded pointer words needs an actual range proof; using
unsigned arithmetic is a different compiler experiment.

**Next ray packet:** keep the ten-word canonical control and the zero-forced
sibling-shape control side by side. A new proposal must explain either the
required phase-one/phase-two partition change, or the complete paired
assignment on the stock shape. Record the causal interference/numbering change
before scoring its register names. No new admissible source lever was proved
in this research pass; do not restart the exhausted ladder, reread, carrier
or empty-test families under new names.

## Interpreter: solve handler-local phase behavior before global naming

Fresh handler lengths remain short by one word at `0x40`, `0x6A`, `0x79`,
`0x7B`, balanced by excess words at `0x41`, `0x60` and `0x78`.
The loop head/latch already have the recorded target topology. The latest
source's `0x7A` is shape-equal but still has two naming differences under the
current handler comparator; an older "exact handler" claim is not a current
whole-function proof.

The most discriminating retained lead is `0x7B`: a narrow-carrier control
produced the target's store/copy order but retained an extra narrowing
instruction. Earlier renaming products and ordinary chained assignments did
not reach both requirements. This points at **when** the assignment becomes
redundant: front-end folding, optimizer copy propagation, and final emission
must be distinguished. The report that the copy is dead does not imply an
extra runtime operation belongs in the source.

**Interpreter history correction:** the October 11 handoff already proves
UGEN-to-assembler copy retention and the address-priority change. Those older
sections were missed when this research proposal emphasized the latest tail.
Fresh phase captures confirm an existing mechanism; they are not its discovery.
The narrow-carrier positive-control claim also needs reproduction on the
current source before it can guide a new candidate.

**Interpreter verification packet:** after reconciling ownership, reproduce the
canonical and narrow-carrier controls in the same full TU. Capture
the actual CFE, UOPT, UGEN and assembler boundaries. Identify the last phase
where the extra narrowing exists and the phase retaining the copy. Only then
test a defined, range-proved identity that simplifies at that later phase.
Use handler shape/length plus full-function aligned buckets; monitor `0x40`,
`0x6A`, `0x79` and the loop-wide constant/address pieces for collateral
changes. Do not trade a lower positional score for damaged handler geometry.

## External research and limits

The current public JFG default branch was checked at
`efd5abb1c79636e297b831f7c2d5bf47eac39c0c`. Its
[hit module](https://github.com/Ryan-Myers/Jet-Force-Gemini/blob/efd5abb1c79636e297b831f7c2d5bf47eac39c0c/src/hit.c)
and relevant
[track](https://github.com/Ryan-Myers/Jet-Force-Gemini/blob/efd5abb1c79636e297b831f7c2d5bf47eac39c0c/src/track.c)
and
[animation](https://github.com/Ryan-Myers/Jet-Force-Gemini/blob/efd5abb1c79636e297b831f7c2d5bf47eac39c0c/src/anim.c)
entry points remain assembly placeholders. This check yielded no new C donor;
it does not exhaust all branches or unpublished work.

The public IDO decompilation's
[spill allocator](https://github.com/n64decomp/ido/blob/d068e439f52615763a3facd6944873899ebad2fd/src/uopt/uoptreg2.c)
provides a readable account of slot assignment from region conflicts.
Its
[input handling](https://github.com/n64decomp/ido/blob/d068e439f52615763a3facd6944873899ebad2fd/src/uopt/uoptinput.c)
also ties `outpar` to special argument-memory handling: it is not evidence
that taking an ordinary C local's address supplies an arbitrary substitution
barrier. That project primarily targets IDO 7.1; it guided inspection only.
The new graph result above is measured in the project's actual IDO 5.3
pipeline, with fidelity controls, rather than assumed from 7.1 source.
No external game implementation was adopted.

## Evidence, validation and continuation

Private evidence is retained in the coordinator checkout under
`build/research-final-20261017/`: stock objects and baseline summaries,
named Ucode capture, slot records, expression report, graph observer and
analysis scripts, graph JSON, OFF/ON fidelity receipts, stack-access map,
and the fresh interpreter handler comparison. Generated compiler source,
objects, traces and extracted instruction material remain ignored.

Commands exercised: `tools/progress.py`, `tools/lane_status.py`, configured
`tools/fast_score.py` builds, `tools/score_symbol.py --object`, the aligned
scorer, `tools/handler_diff.py`, `tools/web_report.py`, workbench capture and
fidelity, and private slot-graph collection/validation. A debugging-listing
attempt failed in the stock host support's `wrapper_ecvt` assertion; that
incomplete listing is not used as proof. The read-only observer was generated
from pristine source instead and passed its controls.

No full-ROM rebuild was needed for unchanged game source and no matching
credit is claimed. Documentation gates are run for this report's local commit.
Before C experimentation, reconcile the existing lane handoffs and obtain the
ordinary zero-exit assignment gate with current pins. Use one 60–90 minute
causal packet per owned target, reserving the closing grace for evidence and
following ADR 0018's information-based stopping rule.

Priority: collision first, because instruction shape is already exact and its
storage obstruction is now proved; interpreter second, using the positive
phase control; ray query when a new defined source mechanism can satisfy its
paired allocation constraint. None is promised to close in one edit.

## SOL execution follow-through

The following results supersede the proposed packets above where they overlap.
The reconciled Grok handoffs and fresh assignment authorizations were integrated
before the three SOL workers started their isolated lanes.

The collision packet reached a stronger diagnostic than the earlier
204-word naming residual: its controlled compiler output reproduces all 370
owned words, the target frame and every stack access. The coordinator
independently scored the frozen object at zero raw and masked differences and
checked all three relocation offsets, types and symbol identities. This is
**not a match**: the result still requires three source-line retention
controls, five allocation-split controls and one priority adjustment. The
source-line controls are not necessarily single destinations; the embedded
factor/discriminant assignment requires destination-specific analysis.
One split control is also consumed more than once. The final private phases
extend previously instrumented IDO 5.3 sources. With controls disabled, their
text, data, rodata, symbols and relocations agree with stock; whole-file
metadata differs. These phases are distinct from the pristine-source observer
used for the initial spill-graph measurement.

Two source changes resolve the earlier UGEN discrepancy under those controls:
same-type casts on the later factor uses establish the needed child
orientation, and putting the negated-coefficient assignment before the doubled
coefficient fixes the earlier operand-release order. Three of the original
six source-line retention controls were then replaced by defined source
changes while preserving diagnostic zero. Removing each remaining allocation
control still loses that result. The ordinary compiler does not produce an
improvement over the canonical 70-word residual.

The natural-retention experiments explain the remaining obstruction more
precisely. Killing an operand can prevent substitution, but its original
allocation generation or call-crossing lifetime can be lost. Both direct
input rereads and named restores preserved the desired retention predicate
while changing the required coordinate ownership. Saving the coefficient
before killing it retained the square but transferred the cross-call
obligation to another owner. A cast at the first comparison and a cast on its
factor did not change the measured evaluation weights. Ordinary sibling
scopes do not reuse scalar homes in the inspected CFE. These routes are
closed on the tested anchor; none is evidence that a stock match is impossible.
The next packet needs a concrete source mechanism that preserves the required
owner generations while replacing a remaining compiler control. See the
[collision handoff](matching-triage-handoffs/func_80054B3C.md).

The ray packet reproduced both forced controls and modeled the coupled
allocation decisions. Its faithful replay accounts for the observed cost
decisions; neither a priority change nor one conflict-edge deletion supplies
the required complete assignment. The small abstract deletion solutions would
remove conflicts whose source live ranges still overlap. This excludes those
specific mechanisms, not every possible C spelling. No new source candidate
was adopted; the stock residual remains ten words. See the
[ray handoff](matching-triage-handoffs/func_80011980.md).

The interpreter packet confirmed the older assembler copy-retention mechanism
and corrected the research-history attribution above. Its new controlled
experiment separates a scheduling barrier from the address-priority change:
retaining the desired local copy still changes the surrounding function, and
the diagnostic allocation adjustment is not an untouched-compiler source
solution. Other short handlers lose their desired structure at different
compiler phases, so the assembler mechanism cannot explain them all. No new
source candidate was adopted; the stock aligned residual remains 261. See the
[interpreter handoff](matching-triage-handoffs/func_800517E0.md).

The coordinator's isolated checkout passed the full ROM rebuild after
regenerating its fresh overlay aliases. The result has the configured SHA-1
`507341c0a40ca3e9a7cee969b396ee53facfb548`. The tooling suite passed all 119
test files; documentation, clean-room and scoreboard checks also passed.
These validate the unchanged canonical game source and tooling, not any
forced compiler diagnostic. The main checkout's pre-existing campaign-note
edits were preserved. No new matching credit or publication follows from
these research and handoff commits.

## 2026-10-18 public-branch follow-up

A read-only GitHub inventory covered the eleven forks returned for the public
[JFG repository](https://github.com/Ryan-Myers/Jet-Force-Gemini/forks): twenty-four
branch rows, twenty distinct commit heads. The three relevant files,
`src/anim.c`, `src/hit.c` and `src/track.c`, yielded fifty-seven snapshots;
all three missing paths belonged to the `queueRAM/none` branch.
Immutable URLs, hashes, inventories and source diffs are retained in ignored
`build/research-public-20261018/`.

No new candidate body was found in those snapshots. The animation variants
contain the existing bitstream helpers or assembly declarations; the command
processor remains undecompiled. The hit variants contain no C bodies. Track
variants with the current body set differ in rendering trace and display-list
source, including the already-merged
[JFG track contribution](https://github.com/Ryan-Myers/Jet-Force-Gemini/pull/23);
older variants have fewer bodies and supply no new ray-query reconstruction.
This excludes these files at these heads, not all public scratches or other
repositories. No external source was adopted and no matching credit changed.
