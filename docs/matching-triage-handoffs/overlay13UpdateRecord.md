<!-- plateau-handoff:overlay13UpdateRecord:start -->
### `overlay13UpdateRecord` plateau handoff

- source: `src/overlays/o013/overlay13ProcessRecord.c`
- score: 70/161 words
- frame: 0x20
- relocations: 5
- first mismatch: +0x4C
- summary: Countdown semantics reconstructed from the listing (old-count tests, count-exit reload, promoted timer, walking vertex pointer, void); 70 at delta 0 remain in the preheader load order, the fade exit's delay slot and the vertex block's schedule.

- geometry: Target and configured C remain exact at `0x284`/644 bytes/161 words with `0x20` frame; the owned Overlay 13 range is `+0x284..+0x508`, ROM `0x186ED9C..0x186F020`, followed by `overlay13ProcessActive` with no padding.
- ABI/flags: The configured candidate is `s16 *overlay13UpdateRecord(Overlay13Record *, s32)` under overlay game-code `-O2 -mips2 -32` and the canonical symbol-redefine/trim postprocess.
- relocation proof: Target and candidate each contain five records with all five offsets/types aligned. Four stable/effective identities align; the `R_MIPS_26` call at `+0x20` remains unresolved because no canonical `overlay13Prepare` definition/alias or exact sibling witness exists.
- diagnosis: Workbench reproduces 96 raw/masked positional differences from `+0x20`, with 63 opcode differences, 31 register differences, and 92 alignment gaps.
- caller/donor result: One Overlay 13 runtime call at `+0x550` and the export authenticate the entry. Donors remain weak: Conker `func_151A6350` leads at 0.0588 Jaccard and supplies no source or relocation witness.
- maintenance result: Current proxy tooling yielded no genuine lever, so no C-body, flag, or permutation hypothesis was attempted under the authorization.
- next action: Preserve the fallback pending unique same-overlay call-proxy evidence or another explicitly authorized mechanism. Previously exhausted loop, result-type, pointer-lifetime, flag, and permutation families remain closed.

#### 2026-09-13, lane `j1`: call authentication and semantic counterexample

The current baseline is 161 words, frame 0x20, masked 96, first masked +0x2C:
86 aligned exact, 34 naming, two immediate and 45 structural rows including
six candidate-only and six target-only words. The old 65-word header was
stale and is corrected. Procedure 0 has 66 draws and 244 emissions; full-TU
stock/traced text is identical. Static tables have five candidate versus three
extracted target records. Runtime target ownership has five records.

The call at owned +0x20 is no longer an unknown same-overlay proxy. Mickey's
runtime table binds it to resident +0x32BF0 (canonical address 0x80033040), whose symbol
is SetLinkSlot. The canonical C defines a three-argument signature with
narrow unsigned tag/count fields. Replacing overlay13Prepare's four-argument
proxy declaration/call with that exact signature and symbol changes no owned
instruction, draw count, per-line count, emission total or draw order. This
source-authentic call correction is retained; no shared header is changed.
Fresh function_preflight.py is complete: all five candidate and runtime
records align in offset, type, stable identity and effective identity, with
static proof and no unresolved records. The former relocation blocker is closed.

Reading the target control flow also reveals a semantic deficit hidden by the
historical score. The fall loop's entry and backedge test the old tick value
before decrementing; the retained diagnostic decrements before each test.
With state one and one tick, for example, the target executes the fall body
once while the retained C skips it. The target also snapshots the remaining
ticks into the result during each fall iteration, which the retained C omits.
The retained numeric floor is therefore explicitly NON_EQUIVALENT, not a
semantically proved near-match. Its source comment now states that limitation.

Separate private corrections preserve each attempt. Correcting the entry
snapshot alone scores 141 and adds one word; correcting the backedge too
scores 147 at the same extent, with 87 exact, 34 naming, four immediate and
42 structural rows including eleven gaps. Adding the loop-result snapshot
scores 151 with two extra words, 87 exact, 35 naming, three immediate and 43
structural rows including twelve gaps. None changes total draws or draw order:
the guard corrections redistribute emission records and the result update
adds two. A further natural while-loop control retains 66 draws but grows by
four words and regresses; it also makes the gravity/target reads loop-local,
so it is not an adopted equivalence proof.

Stop early under ADR 0018 after the authenticated ABI correction and measured
countdown/result controls, alongside the prior loop/lifetime negatives.
Do not repeat the unresolved-proxy diagnosis: it is resolved by runtime
identity. The next action is to reconstruct the target's complete countdown
and return-value semantics before solving the coupled floating-point and
result/constant allocation schedule. Retain the guarded numeric floor and
the corrected private candidates as distinct evidence; no variant is exact.
No colour sweep, flag lattice or permutation was repeated. Commands: assignment
gate, runtime-table identity decoding, canonical callee/source inspection,
function_preflight.py (complete five-of-five identities),
configured stock/traced compilation, draw_census.py comparisons,
residual_map.py --object --against, finalize_plateau.py and tools/gates.sh.
ROM verification covers the retail assembly fallback only.
#### 2026-09-16, lane s1-a: 96 to 70 at delta 0, the semantics reconstructed off the listing

Baseline reproduced at 96 masked, delta zero, first +0x2C, aligned 86
exact, 34 naming, 2 immediate, 45 structural with six insertions each
way. Six cycles, 60 cells. Retained candidate: 70 masked, delta zero,
first +0x4C, aligned 121 exact, 18 naming, 2 immediate, 25 structural
with five candidate-only and four target-only words. Not matched; the
retail fallback stays.

The j1 reading ("the target snapshots the remaining ticks into the result
during each fall iteration") is corrected: every `or v0,s0,zero` is the
dead copy of a bare `ticks--` truth test, the same artefact that closed
`overlay27UpdateCoordinates` the same day, and the function's one caller
declares it void and discards v0. Spelled `x-- != 0` the compiler builds
an `sltu` boolean instead; spelled bare it copies and tests the old value
with the decrement in the delay slot, as the ROM does. What the listing
says the source is, each measured in isolation:

- Both loops are guarded do-while loops on the old count (`if (ticks--)`
  then `do { } while (ticks--)`), the fall loop leaving through a `goto`
  past the count-exit's `state = record->state` reload (the ROM reloads
  only on that exit), the fade loop through a `break` to a reload common
  to both exits. The `for(;;)` and `while` forms with an inner exit test
  are 116 to 141.
- `state = record->state = 2` (sb then andi) in the fall loop.
- `gravity`, `velocityX` and `velocityY` are locals defined in that order
  before the loop (the FP colours f14/f16/f18 follow first-definition
  order, L106; as expression webs gravity colours first and is wrong),
  and the x update is written before the z update (as1 then schedules the
  z chain first): 9 words.
- The fade loop reads `record->timer` through the field with a forwarded
  reload (`record->timer -= 2; timer = record->timer;`): the ROM stores the
  unmasked subtraction from a ring temp and masks once into the promoted
  register. Every masked-local spelling stores the masked value or masks
  twice; the pure field form is one word short (88 at -4).
- `record->vertexIndex = 1 - record->vertexIndex` with no `index` local:
  the ROM's index is a ring temp (t1) and the chain skips a draw (t3),
  which is the deleted copy of the forwarded reload (L149/L150); the
  declared local is a coloured v1 and pushes the constant-1 web off a0.
- The four vertices are filled through a walking pointer (`v[1] = y;
  v[0] = ..; v[2] = ..; v += 5;`), which uopt folds to its final value: the
  `+48` stays with the pointer's definition before the u32-to-float
  branch and the `+30` lands after it, exactly the ROM's two adds, with
  no probe and no region. Writing the final offsets directly forwards the
  definition across the branch and folds the two adds (91 at -4); a
  discarded read of the pointer or an `if (1) { }` region also holds the
  split (79) but is artificial.

What remains, 70 words in three places: the preheader's four invariant
loads are ordered gravity, velocityX, z, velocityY in the ROM against
velocityX, velocityY, gravity, z here (defining gravity first is 73, so it
is not first-definition order alone); the fade loop's timer-zero exit
stores the count before its `beq` and carries a second `lbu` of
`record->state` in the delay slot where ours moves the store into the slot
(a second explicit reload is merged back by uopt, 70 either way); and the
vertex block schedules the 0.1875f constant first here where the ROM
schedules the index load first, with the `beql`/`lw ra` exit fill
following from that. Next: the vertex block's order is the question with
the most words; the ROM's `mtc1 at,f4` sits ten instructions later than
ours, which reads as the constant having no consumer in its block, i.e.
the u32-to-float conversion's `bgez` being a block boundary the ROM's
uopt respects and ours does not. Try the conversion spelled on an
explicit `u32` local, or the radius computed before the pointer, before
any colour work.
<!-- plateau-handoff:overlay13UpdateRecord:end -->
