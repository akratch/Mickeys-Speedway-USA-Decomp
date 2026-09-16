<!-- plateau-handoff:overlay58FinalizePackedStatus:start -->
### `overlay58FinalizePackedStatus` plateau handoff

- source: `src/overlays/o058/overlay58FinalizePackedStatus.c`
- score: 78 differing words
- frame: 0x48
- relocations: 48
- first mismatch: 0x18
- summary: shift before mask, the extended test flipped and every section's i/count/selected order take 88 to 78; a probe-free form has the ROM's mode-0 loop shape but colours the packed-status base t0 for t3.
- assignment base: `ccbd4a78b29afb17ad817dd9228f774012b7d9ac`
- owned range: overlay 58 `+0x5554..+0x5A14`, 1,216 bytes / 304 words; the following `+0x5A14..+0x5A20` range is separately owned padding
- baseline: exact 304-word geometry and `0x48` frame, with 178 raw differences, 177 relocation-masked differences, first raw mismatch `+0x8`, and first masked mismatch `+0x18`
- retained score: 127/304 relocation-masked words match; evaluating the desired-rank condition before the loop bound lowers the workbench normalized structural distance from 177 to 176 without changing size, frame, calls, or semantics
- relocation proof: target and candidate each emit 48 records; 42 offsets/types align, but all 48 candidate identities remain unresolved and zero stable identities align, so `function_preflight.py` correctly returns partial and fails promotion closed
- mismatch proof: the retained aligned view classifies 125 register, 62 structural, seven schedule, and nine constant sites; the residual is broad rather than a near-exact allocator-only tail
- attempts: ten bounded source hypotheses covered loop-condition order, explicit pointer/rank carriers, extended-mode nesting, declaration order, captured-player reuse, stack-home ordering, next-count lifetime, and `for`/`do` loop forms; the complete 119-row flag lattice found no exact result and tied canonical `-O2 -mips2 -32` for best at 177 masked differences
- retained improvement: preserve the desired-first first-loop condition; every other source form regressed geometry, frame, or difference count, and no generic permuter was run
- next action: reopen only with new evidence for the stack-home/declaration web and the mask/shift carrier allocation, together with authenticated candidate relocation identities; do not repeat these ten forms or the completed flag lattice
- JFG: no credible insertion point was found. The pinned JFG overlay donor row is `none`; the nearest JFG skeleton, `func_overlay_3_00304968_1ED9E48`, has only 0.032 similarity and supplies neither a source analogue nor reusable code
#### 2026-09-11, lane f10-mid: the memory-resident set differs, which no home order reaches

Still 88. The function contains calls; the instrumented uopt (text confirmed
identical) records 29 p1 decisions and no p2 records, so declaration order
decides no colour here. The closure's "stack-home order" axis is real for
homes ([L99]) but the census shows the sets differ, not the order: with frame
0x48 on both sides the candidate is resident at 24, 60 and 68 and the target
at 28, 36, 48 and 64, three items against four. Which webs are left in memory
is a p1 colour-or-split decision, so the variable is the save ratio of the
web the target spills and the candidate colours; the p1dec records are in the
lane log and were not joined to symbols this pass. The six L109 probes already
in the source move exactly these ratios, so the next pass should read the
records before adding a seventh.

#### 2026-09-11, lane p5-ovl: joined p1 records and closed two new probes

The fresh baseline remains 88/304 relocation-masked words, with aligned
buckets 227 exact, 41 register naming, 14 immediate-only, and 26 structural;
the frame is 0x48 and the relocation count is 48. The instrumented object is
text-identical to stock. Its 29 p1 decisions have no p2 decisions; the
webdetail frame ladder identifies candidate resident homes 24(sp) = web 99,
60(sp) = web 84, and 68(sp) = webs 0/299. The target object instead uses
28, 36, 48, and 64(sp), so the discrepancy is a resident-set difference, not
home order. The target disassembly labels the distinct homes as records, loop
count, current, and post-call count; the candidate shares one home for the two
count webs. A post-increment count probe and an early records-pointer probe
both remained exactly 88/304 with identical buckets and were reverted.
Forcing candidate p1 count/current webs to split regressed to 100--272 masked
words, so those controls do not identify the target path. Candidate source and
trace evidence is now preserved; no exact source spelling was proved.

#### 2026-09-13, lane k1: authenticated draw-census follow-up

Fresh configured stock compilation reproduces 1216 target bytes,
size delta 0, 89 raw and
88 masked differences, first +0x18.
Candidate frame is 0x48; the procedure-0 census records
44 draws and 574 emission records. Stock and traced full-TU text
compare identically. Static relocation counts are 48 candidate and
24 extracted target, with 0 identical offset/type/symbol tuples.
These are static measurements; overlay runtime identity proof remains separate.

Paired alignment has 227 exact, 41 naming, 14 immediate and 18 structural
rows, plus four candidate-only and four target-only words. Both frames are
0x48, but the resident home sets differ. The existing source probes already
control save ratios, and the shard's joined records identify candidate homes
rather than a simple home-order permutation.

The recorded count/current split forces, post-increment count, early records
carrier, loop conditions, captured-player reuse and home-order attempts were
reviewed. No new source-supported way to select the target's resident set was
identified outside those controls. ADR 0018 early evidence stop applies with
zero new source attempts. No colour sweep or known-flat count/records probe
was repeated; this is baseline confirmation, not another exhaustion claim.
The first-loop discarded selectedPlayer0 read precedes its initialization in
the retained source. That existing source-level issue also needs a defined
replacement before any semantic promotion; it was not introduced or used as
a new diagnostic in this pass. Runtime relocation identities remain unproved.

The retained C remains guarded by its original assembly fallback. Sources, stock
and traced objects, frame and scalar measurements, draw profiles and aligned
maps remain ignored under build/k1/overlay58FinalizePackedStatus.
Commands: lane_status.py, configured stock compilation, draw_census.py,
residual_map.py --object/--against where compared, finalize_plateau.py and
tools/gates.sh. No executable bytes are newly credited.

#### 2026-09-16, lane s1-a: 88 to 78 at delta 0, and a probe-free shape that is structurally the ROM's

Baseline reproduced at 88 masked (89 raw), delta zero, first +0x18,
aligned 227 exact, 41 naming, 14 immediate, 26 structural with four
candidate-only and four target-only words. Five cycles, 37 cells (one
batch was void: a generator edit moved the base to 280 at +4 and was
caught by the base-reproduction cell). Retained: 78 masked, delta zero,
first +0x18, aligned 238 exact, 39 naming, 11 immediate, 20 structural,
same eight insertion words. Not matched.

Three orderings, each read off the listing, semantically inert and
additive on the probe-bearing source:

- `shift` defined before `mask` in every arm. The ROM sets shift (v1)
  before mask (a0) and colours them in that order; ours had mask v1,
  shift a0. First-definition order (L106): 88 to 86.
- The extended-mode test written `== 0` with the 0x1C0/6 arm as its then
  arm. uopt hoists the else arm's two constant assignments above the
  branch and jumps to the join when the condition is false, so the arm
  that appears first in the object is the else arm: the ROM has 0xE00/9
  first and `bne t7,zero`, ours had 0x1C0/6 first and `beq`. 88 to 87;
  with the first, 81.
- Every mode section defines `i`, then `count`, then its selected player
  (mode 2 then `equalFourCount`), which is the ROM's `or v1,zero,zero;
  or a1,zero,zero; or a3,t0,zero` prologue in all three sections. 81 to 78.

Computing `current` before the `desired < 3` return is 94 (worse); a
boolean local for the first loop's compare is inert at 88 and 78.

The finding that matters for the next lane is what the six probes are
doing. Removing them all is 173. Removing them and re-reading the selected
player from `records[0].player` (no `volatile`, no copy of `player`) is
191 -- worse in count -- but the mode-0 loop is then instruction for
instruction the ROM's: `i`, `count` and the selected player materialised
by three `or`s (the third is the forwarded second load, `or a3,t0,zero`),
the `i != selected` compare first with the address shift in its delay
slot, a plain `beq`, and the count exit. The retained source's `volatile`
reload inside that loop is not in the ROM. The residual of that clean
form is 129 naming rows of which 44 are one web: the packed-status base
address, coloured t0 there and t3 in the ROM, with the ROM's t0 and t1
held by `player` and the constant 4. The probes exist to pin that
ordering on the wrong loop shape; the plain copy `selectedPlayer0 =
player` is propagated away and loses four words (272 at -16), so the ROM's
selected player is a distinct symbol holding the second load.

Next: on the clean re-read form, read the p1 ladder (29 decisions, no p2)
for the base-address web against `player` and the constant-4 web and find
what lowers the base's save below theirs or forbids it t0 and t1; then
retire the probes one at a time against that form. Do not add a seventh
probe.
<!-- plateau-handoff:overlay58FinalizePackedStatus:end -->
