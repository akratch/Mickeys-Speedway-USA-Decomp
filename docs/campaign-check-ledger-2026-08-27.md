# Campaign check ledger: post-research spike

Date: 2026-08-27

Baseline: `campaign/unchain` at `bcae0207`

Disposition: handoff ledger; raw generated evidence remains ignored under
`build/`

This ledger executes the bounded checks opened by the lessons in Chris
Lewis's [84-day N64 decompilation campaign](https://blog.chrislewis.au/decompiling-a-nintendo-64-game-in-84-days/)
and adopted in ADRs 0011 and 0012. It is a report of measured scheduling
evidence, not a matching-credit ledger. ADR 0001 remains the only definition of
a match.

## Executive verdict

- The one-shot raw m2c hypothesis is exhausted for the present bare-fallback
  corpus. All 89 eligible functions were attempted; 16 scratch TUs compiled;
  none had exact text. There is no m2c promotion queue.
- The known-source path is still high value. Seven source-built donor objects
  are low-friction promotion candidates, and the remaining Transfer Pak and
  EEPROM cohorts now have small version/flag/TU matrices instead of broad
  searches.
- Same-ROM similarity is useful only as a narrow scheduling oracle on this
  tail. Five targets exceeded a conservative score of 0.25 and none exceeded
  0.50. The two best new leads are `func_80055B24` and `func_80055F64`.
- Cross-lane inspection found no pending exact-match claim. One old
  `func_800498FC` commit is explicitly contradicted by its descendant and must
  remain a recoverable hypothesis, not credit.
- The two-arm compiler-feedback bakeoff used the same tree, target order,
  attempt cap, and evidence fields. Its results are recorded below.

## Scope and baseline

The starting tree reported:

- 1,001 / 1,464 functions matched to byte-identical C;
- 348,376 / 947,796 whole-program text bytes resolved;
- 459 parsed `GLOBAL_ASM` fallbacks: 370 guarded by an existing
  `NON_MATCHING` candidate and 89 bare fallbacks;
- 374 rows in `config/nonmatching-ranking.us.json`, of which 372 had a
  generated result and two had recorded raw-TU isolation failures; and
- no pending exact claim from committed `lane/*` refs.

A direct text search finds 500 `GLOBAL_ASM` occurrences in C, but that count
includes forms which are not distinct eligible bare functions. The parsed
459-function inventory is the campaign denominator; the 89-function subset is
the m2c denominator. These numbers must not be interchanged.

The ranking snapshot is also not a live ownership ledger. A conservative
reconciliation found 319 confidently live rows, including 22 exact-size
register-only rows, one schedule-only row, three relocation-mismatch rows, and
43 exact-size rows within ten differing words. Recompute a target immediately
before assignment.

## Check 1: report-only m2c sweep

`tools/m2c_sweep.py` now performs this check without editing canonical source.
It inventories fallbacks, obtains the owning TU's effective Make flags,
provides m2c with TU context when parseable, discovers separately extracted
jump tables, compiles a complete scratch TU, and compares function text and
relocation identity. All children run sequentially at low priority.

### Results

| Stage | Count |
|---|---:|
| Parsed fallbacks | 459 |
| Existing guarded candidates excluded | 370 |
| Eligible and attempted bare fallbacks | 89 |
| m2c drafts generated | 89 |
| Scratch full-TU compileable | 16 |
| Scratch text exact | 0 |
| Scratch text plus relocation exact | 0 |
| Canonical full-TU, linked, or ROM exact | not attempted |

The other 73 drafts failed compilation: 32 bad operand/type inferences, 16
prototype/call mismatches, seven non-pointer dereferences, six runs exceeding
IDO's 30-error limit, five type conflicts, three undeclared identifiers, two
syntax errors, one IDO internal compiler error, and one residual compiler
error.

Every compileable draft had the wrong text size. The closest still differed
in 91 words. `wakeDraw` independently reproduced its relocation records, but
its text and size differed, so it remains non-exact. Seven legacy K&R TUs
could not be parsed for m2c context; those drafts were still inserted into and
compiled as complete scratch TUs with their real flags. Jump-table inputs for
`func_800517E0`, `func_800563B4`, and `func_800103D4` were recovered from
Mickey's extracted rodata.

Disposition: retain compileable drafts under `build/m2c_sweep/` as draft aids,
but do not assign a promotion lane from this run. Re-run only after a material
header/type improvement, a relevant m2c upgrade, or a changed bare corpus.

## Check 2: known-library source matrix

The live baseline has 134 `src/libultra/*.c` files, no libultra
`GLOBAL_ASM`/`NON_MATCHING` scaffolds, and 20 verified original libultra
assembly units (24 total across the program). The older `docs/modules.md`
wording that still calls `contramread` and `contramwrite` blocked is stale;
both are exact C now.

The remaining strict SDK/libultra tail is 14 probable original-source TUs over
0x3100 section bytes: five source-built anchors, four Transfer Pak bodies, and
five EEPROM TUs containing eight functions. Three adjacent JFG maths TUs and
`osBootRamTest` extend the source opportunity to 18 probable TUs over 0x3650
section bytes. These are section spans, not credit: padding, data ownership,
relocations, and the linked range still require independent Mickey proof.

### Direct source-first queue

Seven donor source objects already have exact whole-text evidence and should be
attempted one coherent TU per lane:

1. `math_atan`, `math_acosf`, and `math_arc` from JFG's MIPS II maths group;
2. `gbpakcheckconnector` and `gbpakgetstatus` from PD with their measured donor
   flag groups;
3. `rmonprintf` from DKR/JFG, retaining the established name because identical
   bytes do not distinguish `rmonPrintf.c` from JFG's `syncprintf.c`; and
4. `osBootRamTest` from the JFG US C path.

The maths group still needs its aggregate data/rodata ownership and 32 text
relocations proved. Every item still needs configured Mickey compilation,
exact relocation identity, linked-range comparison, and full-ROM proof before
promotion.

### Transfer Pak: four-row first matrix

Call surfaces and exact boundaries divide the residue into `osGbpakInit`,
`osGbpakPower`, `osGbpakReadId`, and `osGbpakReadWrite`. Run only:

1. PD init with its O2/MIPS II/R4300/loop-unroll-disabled donor group;
2. PD power with the same group;
3. BK `VERSION_K+` read-ID at O2/MIPS II/R4300; and
4. BK `VERSION_K+` read/write at O2/MIPS II/R4300.

The K+ hypothesis is stronger than a generic donor guess: read-ID has the
distinctive reset/retry call surface and is 0xA0 larger than the currently
built BK-I object; read/write is 0x10 larger than PD and the K+ source adds the
otherwise absent zero-size guard.

### EEPROM: five-row first matrix

Call-graph evidence identifies the former unidentified 0xB50 corridor as five
original-source TUs: long-read, long-write, probe, read plus packer, and write
plus packer plus `__osEepStatus`. Preserve those boundaries. Splitting the
exact DKR `VERSION_G` status body from its original write TU merely because the
current YAML does so risks relocation and data-order drift.

Run only:

1. BK long-write at O1/MIPS II/R4300;
2. BK long-read with the long-write delay idiom at the same flags;
3. BK version-L probe at its donor O2/MIPS II flags;
4. BK-I/pre-J read at O1/MIPS II, then DKR-G read only if helper shape misses;
   and
5. DKR-G/pre-J combined write/status at O1/MIPS II.

Diagnose those results before trying Mickey's established O2/g3/MIPS II group.
Do not expand this into a flag cross-product.

### O3 pipeline queue

`xprintf` and `xldtob` had exact DKR/JFG sections and source-identical donor
bodies. This queue item is now resolved: direct IDO 5.3 at O3/MIPS II/R4300
reproduces both complete objects, including initialized sections and
relocations. The canonical recipes invoke `tools/ido/cc` directly because
asm-processor's CLI has no O3 mode. No custom interprocedural wrapper or source
permutation was needed.

The JFG reference farm was observed at `c75c270d`, ahead of the documented
pin `c82afff`. Relevant bodies appeared unchanged, but promotion evidence must
use the pin or explicitly refresh it.

## Check 3: Mickey-vs-Mickey and committed-lane similarity

The conservative masked-skeleton pass indexed 1,018 matched-C Mickey sibling
records, 796 of at least ten words, and compared them with all 459 fallbacks.
Of 456 targets with a nonzero candidate, only five exceeded 0.25 and none
exceeded 0.50. These are source-shape leads, never equivalence or provenance
evidence.

Highest new leads:

| Target | Closest matched Mickey sibling | Score |
|---|---|---:|
| `func_80055B24` | `func_800557F8` | 0.496 |
| `func_80055F64` | `func_800560D0` / `func_80055D08` | 0.421 / 0.420 |
| `runlinkFreeCode` | `runlinkUnloadOverlay` | 0.288 |
| `func_80003760` | `amSndUnlinkHandleXYZ` | 0.267 |
| overlay 73 `+0xD70` | `overlay96DrawObject` | 0.253 |
| `overlay31CreatePool` | `overlay31CreateRecords` | 0.203 |

The installed coddog 0.6.3 CLI cannot run its indexed `match`/`cluster` path
because Mickey has no coddog `decomp.yaml`. The audit reused
`tools/skeleton_scan.py`'s conservative masking in an ignored report-only
scanner instead. Building an index is useful only if the masked scan's top
leads prove productive.

`tools/lane_status.py --pending-only` found no pending match. Full committed-ref
inspection found an old `lane/trace-1` subject claiming `func_800498FC`, but
descendant `7dbdf31f` explicitly withdrew exactness and restored
`NON_MATCHING` plus `GLOBAL_ASM`. Commit `63826f8b` is a hypothesis seed only.

## Check 4: controlled model-routing bakeoff

Both arms began at `bcae0207`, checked the same committed-lane state, and
worked the same target order:

1. `frontSetScreenMode`: compact P! allocation plateau; and
2. `func_80012574`: P! FP-web/stack-home plateau whose earlier permuter zero
   was proved false when honest stack differences were enabled.

Each arm had a 40-minute soft budget, an eight-minute handoff reserve, a cap
of ten coherent attempts per target, the configured full-TU/workbench loop,
and one low-priority compiler process. The permuter was excluded so this
measures model-guided compiler feedback rather than search throughput.

| Route | Target | Attempts | Baseline | Best | Active wall time | Verdict |
|---|---|---:|---:|---:|---:|---|
| inherited | `frontSetScreenMode` (pre-cleanup) | 10 | 5 / 32 | 5 / 32 | about 5 min | historical diagnostic |
| inherited | `func_80012574` | 10 | 7 / 57 | 7 / 57 | about 5 min | plateau |
| Sol/xhigh | `frontSetScreenMode` (pre-cleanup) | 8 | 5 / 32 | 5 / 32 | about 3 min | historical diagnostic |
| Sol/xhigh | `func_80012574` | 10 | 7 / 57 | 2 / 57 | about 5.5 min | diagnostic, rejected source |

Both routes reported 0 / 2 exact. Their attempt counts, matrices, objects, and
traces do not survive, so the table is historical prose rather than proof of
route exhaustion. The retained prior-layout `frontSetScreenMode` C object is
size-exact and frameless with all six relocations exact; its five sites form the
same mode-web `v0`/`v1` inversion. Reopen with current-layout V0, a retained
119-flag lattice, one allocator trace, and only trace-selected natural forms.

Fresh measurement corrected `func_80012574`'s older six-word note to seven
raw differing words. The inherited route did not improve it. Sol/xhigh used
two empty conditional probes to align the complete integer and FP allocation
webs, leaving only the call-crossing projection store/reload at `sp+0x30`
instead of the target's `sp+0x38`. Text size (57 words), frame (`0x48`), and
the sole `sqrtf` relocation are exact. The two-word result is diagnostic only:
the empty guards violate the project's no-invented-guards rule, so commit
`0503fdaa` is rejected from integration and the honest seven-word source stays
canonical. Its next action is an F-route stack-home/declaration-order trace or
original local-layout evidence.

Sol/xhigh won this two-target pilot on diagnostic residual reduction, while
exact count tied and integration yield remained zero for both routes. The
cohort is too small and too mechanism-specific to change ADR 0009 or establish
a general model ranking. It does justify routing this stack-home plateau to an
instrumented F-route and expanding a future bakeoff with several structural
and relocation-bearing targets.

The result must be treated as a small controlled pilot, not a general model
ranking. Exact count is primary; honest differing words, mechanism quality,
attempts, and wall time are secondary. A scratch zero is not an exact result.

## Expert escalation queue

The queue below combines explicit multi-pass plateaus, hard mechanism classes,
close live rankings, and the new same-ROM leads:

1. `func_80055F64` — strongest reusable same-ROM family;
2. `func_80055B24` — strongest single Mickey sibling;
3. `overlay40FadeRecords` — three-word register-only live ranking;
4. `overlay3FindClosestObject` — four-word register-only live ranking;
5. `frontSetScreenMode` — compact P! plateau and bakeoff target;
6. `overlay40AddEntry` — four-word register-only live ranking;
7. `overlay20UpdateObjectResource` — historical two-pass claim; canonical Git retains only the isolated scalar, while one stale local V0 object lacks source/build provenance and no route-exhaustion artifacts survive;
8. `overlay74Update` — historical six-word pre-cleanup diagnostic; current
   policy-clean C is uncompiled;
9. `overlay2SplitRegion` — W-route scheduler expert or park;
10. `func_80012574` — configured 7/57 FP-web/stack-home plateau; run the
    bounded saved-local/declaration-order ladder;
11. `func_800320F0` (`runlinkEnsureJumpIsValid` donor alias) — since resolved
    exact; this dated queue entry is historical only; and
12. `levelInit` — since classified reproof-only after source, 119-flag, and
    bounded-permuter exhaustion; this dated queue entry is historical.

`overlay43SubmitChildren` was a four-word ranking at this ledger's baseline.
It has since been matched and integrated, so it is no longer in the queue.
`overlay20UpdateObjectResource`'s two-pass conclusion survives only as
historical prose; its retained artifacts prove V0, not route exhaustion.
`overlay2SplitRegion` remains the cleaner retained plateau packet.

Every expert packet should contain the exact function identity and ownership,
configured flags/TU boundary, callers and ABI, current candidate and fallback,
target/candidate size and relocation surface, honest baseline and best scores,
first mismatch, workbench mechanism and `next:` lever, attempts already made,
nearest permitted donor or Mickey sibling, provenance status, and one concrete
next experiment. Never include aligned instruction rows or ROM words.

## Recommended campaign order and stop rules

1. Integrate the report-only m2c driver. Record raw m2c as closed until one of
   its explicit rerun triggers occurs.
2. Give each of the seven exact-source anchors an independent promotion lane.
   Exact donor text selects source to try; it does not skip Mickey's proof.
3. Run the four-row Transfer Pak matrix, then the five-row EEPROM matrix.
   Stop each cohort when the named source/conditional/flag rows are exhausted
   and record a compact plateau.
4. In parallel with those bounded library checks, work the two strong Mickey
   audio-family siblings and the compact F-route expert queue.
5. Resolved: direct IDO 5.3 O3 promoted `xprintf` and `xldtob` exactly; retain
   those scoped recipes and do not reopen the flag search without new bytes.
6. Use the bakeoff result to choose the next expert route, but rerun on a larger
   fixed cohort before changing ADR 0009.
7. Recompute queue, atlas, provenance, and scoreboard numbers after every
   integrated exact result. Never carry this ledger's point-in-time counts
   forward as current state.

## Reproduction and safety record

Representative commands:

```sh
python3 tools/lane_status.py --pending-only --json
python3 tools/lane_status.py --json
jq '.queue_size, .resolved, .unresolved' config/nonmatching-ranking.us.json
nice -n 15 .venv/bin/python tools/m2c_sweep.py \
  --fresh --out build/m2c_sweep --timeout 120
```

The source-matrix audit used bounded `tools/skeleton_scan.py scan --region`
queries for the Transfer Pak and EEPROM ranges, a read-only call-target census,
and section/symbol/relocation inspection of existing objects. The similarity
audit used committed Git objects and an ignored masked-skeleton report. Raw
reports, candidates, logs, objects, and scripts remain under ignored `build/`
state and are not part of this commit.

No test runner or test executable was run. No browser, GUI, emulator,
simulator, generated game binary, or ROM was executed. Compilation was limited
to at most two low-priority single-job lanes. No post-compile instruction edit
was used, no hook was bypassed, and no external branch was pushed.

## Handoff state

The durable tooling commit on this ledger branch is `9bf48c46` (original m2c
lane commit `41d7a363`). The inherited bakeoff arm changed no tracked files.
Sol/xhigh commit `0503fdaa` preserves the two-word diagnostic on its own lane,
but must not be merged because its empty guards violate the matching workflow;
the clean-room-safe result and next lever are recorded here and in the updated
resident/triage notes instead.

This ledger commit changes `docs/campaign-check-ledger-2026-08-27.md`, links it
from `docs/campaigns.md`, and corrects `func_80012574`'s stale measurements in
`docs/resident.md` and `docs/matching-triage.md`. The accompanying turn handoff
records the final commit hash and policy-check commands.
