<!-- plateau-handoff:func_8000FAE0:start -->
### `func_8000FAE0` plateau handoff

- source: `src/main/track.c`
- score: 16/62 words
- frame: 0x10
- relocations: 2
- first mismatch: +0x1C
- summary: p2 a0/t0 cycle is count vs bound/coord web order. Declaration order is inert; hoisting z/y/xLower or delaying count regresses. Count stays a0.

- Reopen audit (2026-09-08): the local Jet Force Gemini checkout is exactly
  `efd5abb1c79636e297b831f7c2d5bf47eac39c0c`, with an unchanged tracked
  `src/track.c`. That file retains `trackGetBlock` as `GLOBAL_ASM`; there is
  no newly matched donor C body to adapt for this target. The near-match
  oracle ranks `trackGetBlock` first (300 bytes, masked 4-gram Jaccard 0.0561).
  The donor scan is structural evidence, not a byte-identity or name adoption.
- ABI and ownership: Mickey takes three single-precision coordinates and
  returns a segment index. Its resident range is VRAM `0x8000FAE0..0x8000FBD8`,
  ROM `0x106E0..0x107D8`, with 248 executable bytes and no owned padding.
  Both games scan signed-halfword bounding boxes, but JFG selects distance
  to the vertical midpoint while Mickey accepts containment immediately and
  otherwise measures distance to the nearest vertical bound. Importing the
  donor behavior would therefore change Mickey's semantics.
- Fresh configured full-TU baseline: `-O2 -mips2 -32 -Wab,-r4300_mul` with
  `-DNON_MATCHING`, 62 target and candidate words, 42 identical words, 20 raw
  and relocation-masked differences, first `+0x1C`, and matching 16-byte frames.
  Both relocation records agree in relative offset, type and symbol identity:
  the HI16/LO16 pair for `D_800792E8`. This is compiled nonexact evidence;
  the candidate remains guarded and earns zero new matching bytes.
- Workbench: comparison `allocation-mismatch`; mechanism `allocation`,
  playbook `register-role-audit`, owning pass unknown. The forced-color-oracle
  guide says a trace or authenticated reservation state is required before
  any allocator intervention; neither is available or authorized here.
- Attempt evidence: attempt 1 reordered x/z bound declarations to the target
  load order and remained at 20 differing words. Attempt 2 removed the loop
  condition carrier and regressed to 61 instructions, 27 differing words,
  7 opcode mismatches, and a relocation mismatch; the best candidate was
  restored. The report-only m2c sweep found no eligible bare GLOBAL_ASM because
  this symbol already has a guarded candidate.
- ADR 0018 stopping evidence: the permitted new-donor-C hypothesis remains
  disproved by the pinned source; the only source probe preserving shape was
  flat and the alternative was a measured regression. Repeating declaration,
  flag, allocator, or permuter work would not test the authorized mechanism.
  No claim is made that the function is inherently unmatchable.
- Next concrete lever: obtain a published matching `trackGetBlock` C body and
  re-prove its ABI and vertical-selection differences against Mickey. An
  allocator investigation would need separately authorized mechanism and
  authenticated register-role evidence.
- Evidence retained locally in ignored `build/wb/`: baseline source/object,
  comparison summary, provenance receipt, diagnosis and donor-oracle result.
  Commands: `tools/wb_compare.sh --summary-json func_8000FAE0`, workbench
  `diagnose` on the configured `build_non_matching/src/main/track.c.o`,
  workbench `guide forced-color-oracle` and `guide register-role-audit`,
  `python3 tools/m2c_sweep.py --symbol func_8000FAE0 --fresh`, and
  `tools/finalize_plateau.py` with the measured fields above.

#### 2026-09-12, lane p19-reopen: p2 decisions priced, four naming words closed

The authenticated reopen admits allocator work; the older donor-only stopping
condition does not cover it. Baseline buckets were 42 exact, 20 naming, zero
immediate and zero structural, with zero displacement tax. Defining the x and
z lower bounds before their upper bounds gives 46 exact, 16 naming, zero
immediate and zero structural. Both objects contain 62 words, frame 0x10 and
the same two relocation identities. First mismatch remains +0x1C. The reads
are independent, nonvolatile accesses in the same basic blocks.

The instrumented compiler was derived from the configured full-TU command,
including the multiply flag. Its baseline text is identical with logging on.
The procindex table and TU order identify ordinal 29, whose decisions are p2
only. Thus L142 call arity and L139 save ratios do not reach this leaf.
On the baseline, web 6 holds the segment count in a0; its accepted force to
c7 (t0) alone regresses to 32 because it changes the later colour assignments.
An accepted combined force of webs 6, 10, 13, 17, 28, 32, 46, 48, 52, 65, 67
and 71 onto the observed target colours gives two naming differences at delta
zero. Every requested colour is recorded as accepted in the p2color row.
This prices 18 original differences as p2 colouring; it is diagnostic output,
not a candidate or a match. The remaining pair is the loop-condition carrier.

After the source adoption the census is coherent: count a0 versus t0 at three
sites, coordinate t0 versus a0 at twelve, and condition temporary t9 versus at
at two; one instruction participates in both substitutions. The source lever
needs to change count/coordinate web order while retaining the existing
preheader, frame and loop-condition shape. No ratio adjustment can do that.

Measured negatives: widening the halfword locals and a register qualifier
are text-inert. Inlining the count changes naming without fixing it; inlining
all bound carriers adds five words. Index induction adds one word, while its
natural condition form keeps size but changes structure. Reusing coordinate
carriers is flat; reusing the initial count carrier regresses or adds words.
Moving the bound read to the loop tail adds sixteen words. Early boolean
carriers add a word. The final three boolean-carrier variants are identical
regressions and supply no further information, meeting ADR 0018's stall rule.

The best guarded source is retained. Source, object, direct-score reports,
compile commands and force-acceptance logs are preserved in lane-private
external scratch. Commands include align_symbol, residual_map, register_census,
frame_census and configured full-TU stock/instrumented comparisons.

#### 2026-09-13, lane f1: coordinate deletion and count-width differential

The fresh configured baseline reproduces 248 bytes, zero delta, 46 exact and
16 naming rows, no other buckets or gaps, first +0x1C. Named Ucode authenticates
procedure 29 of 66. Its draw census has seven draws and 124 emission records.
All tested objects pass full-TU stock/instrumented text identity. Existing p2
force receipts and prior carrier/induction/boolean closures were read; no
colour sweep was repeated.

Deleting only the converted-X carrier and using explicit integer conversions
at its comparisons leaves all seven draws and their sequence unchanged. One
emission record disappears at the old conversion definition, but naming grows
from 16 to 21 at unchanged extent. It supplies no draw-order improvement.

Changing the count to unsigned halfword while explicitly interpreting both
comparisons as signed preserves the original count domain. This adds one draw
and three emission records at the nonempty guard, changing draw order. The
aligned paired map improves to four naming, one immediate and one structural
row, but it also has three new candidate-only words near the entry and is
12 bytes too large. The 55-word positional residual is mostly displacement
and is not the reason to reject this geometry.

The corresponding unsigned-word count with signed-word uses removes the
narrowing conversion. It restores all baseline draws, emission counts, order,
bytes and aligned rows exactly. Thus the measured count-order improvement is
coupled to the extra conversion; declared unsignedness by itself does not
supply it. The 16-word guarded baseline remains the best exact-extent source.

Stop early under ADR 0018 after the width control eliminates the last new
hypothesis in this family. The previous source closures cover the remaining
identified condition/induction routes; no broader source impossibility is
claimed. The next useful evidence must change count/coordinate creation order
without a surviving width conversion, and separately remove the condition
carrier while retaining the target's loop shape.

Sources, objects, census differences and aligned maps remain ignored under
build/f1/func_8000FAE0, with mapping evidence under build/f1/track-map.
Commands: configured stock/capture compile, allocator_trace_receipt.py
--map-only, draw_census.py profiles/comparison, residual_map.py --object
--against, finalize_plateau.py and tools/gates.sh. Zero new byte credit.

#### 2026-09-17, lane w5-track: L106 definition-order cells

Follow-on after func_80010900 plateau. Baseline 248 bytes, delta 0, 46 exact
and 16 naming, first +0x1C. Census is one coherent a0/t0 cycle (count three
sites, bound/coord twelve) plus t9/at on the loop compare (two). Leaf, p2.

Declaration order of count vs zInt/xLower/coords/bounds/keepGoing is
byte-identical at 16. Hoisting zInt, yInt, or xLower=0 before count keeps
size and moves to 18. Hoisting all three conversions is 22. Moving count
after bounds/i is 33. Assigning count after xInt, with the first test on
the field, is +12 bytes / 58 words. Dropping the keepGoing local is -4
bytes / 23 words. s32 count is still 16.

The count web is defined before the loop; the bound/coord web's first real
def is inside it. An early dummy def that survives numbering without an
extra conversion is still missing. Best source unchanged.

Commands: score_symbol.py on the definition-order and declaration-order
cells, align_symbol.py, register_census.py, finalize_plateau.py.
<!-- plateau-handoff:func_8000FAE0:end -->
