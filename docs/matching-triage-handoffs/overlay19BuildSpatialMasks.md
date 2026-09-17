<!-- plateau-handoff:overlay19BuildSpatialMasks:start -->
### `overlay19BuildSpatialMasks` plateau handoff

- source: `src/overlays/o019/overlay19BuildSpatialMasks.c`
- score: 9/227 words
- frame: 0x80
- relocations: 0
- first mismatch: +0xC0
- summary: The span base as a non-volatile expression web puts firstItem in v1 and span in a0, 17 to 9; the mask/selector init order and loop 1's tail schedule remain, both as1 order.

- geometry: Target and configured C are both `0x38C`/908 bytes/227 words with frame `0x80`; the owned Overlay 19 range is `+0xF58..+0x12E4`, ROM `0x18761B0..0x187653C`, followed by separately owned 12-byte assembly padding.
- relocation proof: Target runtime and candidate static surfaces both contain zero relocation records; count, type, offset, and identity surfaces are therefore vacuously exact, and preflight is complete.
- diagnosis: Fresh workbench comparison has 63 raw/normalized differences, two opcode mismatches, 61 register differences, and twenty shift-tolerant alignment gaps.
- history: The body remains the original guarded reconstruction from `752cefadbde9a2ffb9128690055650d33e6edcdc`; this target-named evidence refresh changes no executable source.
- authorized maintenance reproof: A second forced configured V0 from authorization tip `ce9f8cd6b8581e4ec3f3014c88ec2a6e66a8d310` reproduces every metric and confirms the separate `+0x12E4..+0x12F0` padding boundary.
- next action: Preserve the fallback pending a genuinely new natural pointer/item-index coloring or independently proved source structure. Prior declaration, lifetime, load-order, alias, loop-variable, flag, and permutation families remain closed.

#### 2026-09-13, lane `j1`: point and scaled-offset draw controls

Fresh baseline reproduces 227 words, delta zero, frame 0x80, masked 63,
first +0x58: 164 aligned exact, 61 naming, zero immediate and two structural
rows without gaps. Procedure 0 has 63 draws and 417 emissions. Both relocation
surfaces are empty. Existing pointer/arithmetic, lifetime and order evidence
was read; no colour sweep was repeated.

Deleting the explicit point carrier and indexing group->points at the selector
read moves two draws off line 32 and adds three on line 33. Draws become 64,
emissions 416; the complete sequence changes. Geometry and frame stay fixed,
but aligned naming rises to 113 and structural to six, with 119 masked words.
The point-carrier route moves the intended lines and is worse in both windows.

Separately, naming the unsigned selector-plus-base offset before its scale
removes two draws from line 33 while adding three elsewhere. This has 64
draws, 422 emissions, frame 0x88 and three extra words. It scores 218;
alignment is 78 exact, 124 naming, eight immediate and twenty structural
rows including the three additions. Deleting the resulting vertex pointer
and spelling the three field reads through that offset removes one emission
record on line 33, but changes no draw count or order and is object-byte-flat.
Thus pointer deletion does not undo the named-offset candidate's spill cost.

The guarded original remains best. Stop early under ADR 0018: the newly
identified carrier deletion and partial-offset routes have measured local
movement and costs; prior lifetime/order controls cover the remaining stated
levers. This is not general source impossibility. Private source/object pairs,
stock/traced full-TU identity, censuses and aligned deltas are retained.
Commands: assignment gate, configured compilation, draw_census.py before/after,
residual_map.py --object --against, finalize_plateau.py and tools/gates.sh.
The verified ROM uses the fallback; no new matching bytes are credited.

#### 2026-09-13, lane l1: measured schedule controls

Fresh procedure-0 baseline is 227 words at delta zero, frame 0x80, with
63 masked differences from plus 0x58 and no relocations. Alignment is 164 exact,
61 naming and two structural rows without gaps. The census has 63 draws and
417 emissions. The existing exhaustive colour and point/offset controls were
read before making a source change.

Changing the mask-table scale literal from signed ten to unsigned ten, while
retaining the unsigned product and all carriers, tests the scale's Ucode type
normalization. Every line's draw/emission count, the complete draw sequence,
and the full stock text remain identical. The scale expression does not move
under this edit; every aligned window is unchanged. Restore the original.
Stop early because this normalization adds no reachable scheduling mechanism
to the previously closed point/offset controls. The remaining question is the
scale-emission order itself; obtain a trace witness for a different expression
structure before reopening that source axis.

Named Ucode mapping and full stock/capture fidelity pass for the baseline and
each retained experiment. Sources, stock objects, scores, frame/relocation
censuses and aligned deltas remain under ignored build/l1/overlay19BuildSpatialMasks. Commands:
configured compilation, allocator_trace_receipt mapping, draw_census profile
and comparison, residual_map object comparison, finalize_plateau, and
tools/gates.sh verify cleanroom check-docs. No matching credit is claimed.

#### 2026-09-16, lane s1-b: the counter's web number, read off the ladder

Baseline reproduced at 63 masked, delta zero, frame 0x80, aligner 164 exact,
61 naming, 0 immediate, 2 structural. The instrumented uopt (identity gate
passed, 228 of 228 words) prices the three contests directly.

The a0/v1 swap in the bin loops is the counter's decision order. Reusing
firstItem as the counter makes web 19: six blocks (it also lives in the span
head), total 10330, save 1721.7, decided after binEnd (web 154) and binStart
(web 160), which tie at 9300 over 5 blocks, 1860. A fresh s16 counter is
9300 over 5 as well, so the tie is broken by ascending web number, and its
first occurrence sits between binEnd's and binStart's: 63 to 38 in one cell.
Its declaration position is inert (three positions, all 38). A probe on the
counter inside loop 1 adds 1000 to the total and moves it ahead of binEnd
(66). Every non-do form of loop 1 puts binStart's first occurrence ahead of
the counter's and loses the numbering (for 70, hoisted-init for 34, while
42).

The t1/t2 swap is xMax (web 45, 3100 over 8, 387.5) decided after xMin (web
62, 3100 over 7, 442.9). An if (xMax) probe inside the vertex loop raises
xMax to 4100 over 8, 512.5, ahead of xMin and behind mask: 38 to 24.

Swapping the mask and selector zero inits reached 19, though the two moves
themselves did not reorder; both probes are load-bearing (dropping either is
52) and moving mask's init after point or vertices is 24. Folding the two
loop-1 inits onto one physical line is 17 (an L59 tie; the reverse order on
one line is 52). Separate lines for loop 1's body statements are 18 to 22.

Retained at 17: aligner 212 exact, 12 naming, 0 immediate, 4 structural,
one candidate-only word at +0x204 and one target-only at +0x1FC.

What remains, with the record that blocks each:

  the span pointer (web 14) and firstItem (web 19) are p2 webs, coloured in
  ascending web number; span is created first (lineage event 5 against 7,
  first reference in the IR, unchanged by declaration order) and is offered
  v1 first at cost 0. The target has span in a0 and firstItem in v1, so it
  either references firstItem earlier or holds v1 with a phantom at the loop
  head. Reading the span through an expression instead of a carrier is 210
  at +24 because the volatile offset is re-read per occurrence; a firstItem
  probe ahead of the span load extends its range over the back edge (210,
  frame 0xC0).

  the selector/mask init order (two rows) is not statement order and not a
  physical-line tie; it survives every placement measured here.

  loop 1's tail (slti before the sign-extensions, the bit shift's move in the
  delay slot) is as1's order; loops 2 and 3 already match with the same
  statements.

Evidence: sources, objects, ladders and side-by-side listings under Git's
common dir, lane-evidence/s1-b/t2. Commands: private direct-cc harness
reproducing score_symbol on the base, score_symbol, align_symbol,
residual_map --object, the instrumented cc with CDX_DETAIL_WEB=all.

#### 2026-09-16, lane s2-a: the span pointer as a type-4 web, 17 to 9

Baseline reproduced at 17 (212/12/0/2, first +0x58); identity gate passed
on procedure 0. Four cycles, 34 cells. Adopted at 9: 220 exact, 4 naming,
0 immediate, 2 structural, first +0xC0, frame 0x80, delta 0.

- p2 colours in ascending web number and both `span` (web 14, created at
  event 5) and `firstItem` (web 19, event 7) are type-3 webs, so a
  declared `span` is always numbered first and takes v1. Dead
  `firstItem` definitions at the loop head (zero, spanCount,
  self-assignment) are deleted before numbering: inert at 17.
- Reading the four span fields through the expression itself, spelled
  with the union's NON-volatile member, is CSE'd to one load and makes
  the base a type-4 web numbered after every type-3 web: it is decided
  after firstItem and takes a0, firstItem v1. The shard's "+24 for the
  expression form" was the volatile member re-read per occurrence; the
  volatile qualifier never produced the loop-head reload (the carrier
  form with the non-volatile member is byte-identical to the base), the
  memory-class struct does. A volatile first read with non-volatile
  others is 207, +8 to +16.
- The remaining rows are as1 order: the mask/selector zero inits (two
  rows), and loop 1's tail (the binIndex/binEnd sign-extension order and
  the slti position, one candidate-only and one target-only word). Loop
  1 as a `for` still loses the counter numbering on this shape (hoisted
  init 26, comma init 26, plain 63). The 16-cell physical-line grouping
  lattice of the tail's four statements, in two statement orders, floors
  at 9; four cells reach 221 exact with one structural row and a
  different residual (10).

Next: the tail is as1's list order among four independent statements on
one line; the lever left is the statement set itself (a `bit` or
`binStart` spelling that changes which instruction is ready first), and
the init pair is a two-move tie the probes may be deciding. Read
`cc -Wa,-R` for loop 1 before the next cell.
<!-- plateau-handoff:overlay19BuildSpatialMasks:end -->
