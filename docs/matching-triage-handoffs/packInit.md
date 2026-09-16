<!-- plateau-handoff:packInit:start -->
### `packInit` plateau handoff

- source: `src/main/saves.c`
- score: 115/115 words
- frame: 0x60
- relocations: 20
- first mismatch: none
- summary: ROM-exact and promoted: 115 words, frame 0x60, delta 0, unforced; the pak pattern is masked again before the bit test, which draws the one ring temp the target spends and as1 folds into the `lbu`.
#### 2026-09-12, lane p23-lastmile5: exhaustive colour evidence

The configured baseline is 115 words, 34 masked differences, delta zero,
frame 0x60, 20 relocations, first +0xA0. The older header's 81 is the agreeing
count. Aligned buckets: 81 exact, 25 naming, zero immediate, nine structural;
there are no surplus or missing words. No source was adopted.

Captured Ucode authenticates procedure 29 of 42. Stock/instrumented fidelity
passes text, data, rodata, symbols and relocations. The complete same-save-kind
landscape covers all 12 coloured webs: 69 compiled and accepted alternatives
at delta zero and one web with no alternative. The exhaustive winners list is
`p1:w19=c7` at 25 and `p1:w30=c7` at 27. The three-cell lattice reproduces both
singles and scores their accepted pair at 28, ten words worse than its additive
prediction. Their overlapping radii make them rivals. The measured diagnostic
lattice floor is 25, while stock remains 34.

Six source forms are retained privately under `build/p23/packInit/`. A named
PFS pointer assigned before or after the rumble pointer scores 37 at equal
instruction count, but grows the frame by eight bytes; its aligned buckets
are 78 exact, 24 naming, three immediate and ten structural. Removing the
second-loop rumble carrier and indexing the global directly remains 34.
Crossing those three shapes with a word-sized controller bound gives 37 in
all three cases. These final three cells neither improve the retained residual
nor establish a new identity, meeting the stall rule. No frame-growing form
was adopted for a smaller local register residual.

The remaining source question is the lifetime and materialization point of the
PFS array base relative to the controller bound, followed by the success-store
schedule. Introducing its local pointer does not move that base as required;
inlining the rumble pointer is commoned back into the existing address range.
Neither force winner can fix all structural rows on the current shape.

#### 2026-09-12, lane p24-recipe: generated controller bit is informative but short

The stock baseline remains 115 words, delta zero, frame 0x60, twenty
relocations, 34 raw/masked differences and first mismatch +0xA0. Its aligned
buckets are 81 exact, 25 naming, zero immediate and nine structural rows.
The existing authenticated 12-web landscape and rival-force floor 25 were
reused; no source change was adopted.

Deleting the controller-bit carrier and writing one shifted by controllerIndex
at its uses yields 113 words, delta minus eight, with 35 positional differences.
The aligned result is substantially different: 96 exact, one naming, one
immediate and fifteen paired structural rows, plus two target-only words.
The naming residual largely disappears, but the loop now computes the bit
from the index instead of preserving the target's byte-sized recurrence. This
is a retained structural diagnostic, not a matched or equal-geometry candidate.
Casting the generated bit to u8 restores 115 words but yields 57 masked and
41 paired residual rows, with two surplus/missing pairs; it does not restore
the target recurrence. A word-sized declared bit is eight bytes short at 73.

Deleting the bound carrier scores 76 at delta minus four. Replacing both
rumble-pointer uses with indexing scores 57 at equal geometry but changes the
first loop and frame displacements. Keeping a separate reset pointer while
indexing only the second loop is flat at 34. Reusing ret for the reset counter
gives 38 at delta zero, changing only four previously agreeing entry rows.
Preloading the success status through ret, both on separate lines and grouped
with its stores, is flat at 34; propagation restores the original schedule.
All sources, objects and aligned per-window deltas live privately under
build/p24/packInit. One mistyped field in the reset probe was corrected before
its successful compile; the failed compiler artifact is retained separately.

The final counter and two status-preload forms establish no better residual or
new target identity, meeting ADR 0018. The named source question is a bit
recurrence with the target's extra temporary draw and PFS-base materialization,
followed by interleaved success loads/stores. Deleting the bit explains much of
the naming symptom, but it also removes the byte-mask recurrence that the
shipped loop retains. Do not count its improved aligned diagnostic as a match.

#### 2026-09-13, lane g1: all structural rows close, leaving one coherent ring phase

The assignment gate returns base-only. Fresh named Ucode maps packInit to
procedure 29 of 42; stock and traced full-TU text agree. Baseline: 115 words,
zero size delta, frame 0x60, twenty relocations, 34 masked differences, aligned
81 exact, 25 naming and nine structural rows. The census has fourteen total
draws, including two at the bit predicate. The earlier two-draw description
refers to that line, not the complete procedure. The recorded rival-force
landscape is reused, with no new colour sweep.

The retained source makes three changes, each measured separately:

- Fold the index, bit and bound initializations together with the loop/guard
  opening onto one physical line. This preserves the fourteen draws and their
  order, closes the PFS-base completion window, and improves 34 to 30 masked
  words: 85 exact, 24 naming, six structural. Fifteen emission events move to
  the common source line; no scratch draw is added.
- Place the independent bit recurrence before the index increment. This
  preserves the draw sequence and improves to 29: 86 exact, 25 naming, four
  structural rows. The byte-mask scheduling pair in the loop tail closes.
- Generate the final motor-success status address from the indexed rumble array
  instead of the declared pointer. This ties 29 while closing every structural
  row: 86 exact, 29 naming, zero immediate/structural, no unique offsets. The
  same two draws remain at that update; eight emission events are added. This
  changes address identity and permits the desired load/store interleaving.

The last form is retained over the equal-score form with four structural rows.
All 29 differences now follow one ten-register rotation, at 100 percent mapping
coherence in one window. First mismatch moves from +0xA0 to +0xB0. The target's
first predicate load is one draw ahead; the source question is now a folded
draw before that load, with no added instruction or changed agreeing producer.
The old PFS-base and success-store blockers are resolved on this source shape.

Individually retained controls delimit that question. Shift spelling for the
byte recurrence, folding just the two success stores, and deriving the initial
bit from the known-zero index are byte-flat on the incoming source. Explicit
byte projection of the predicate adds one draw and one executable word; byte
projection plus an explicit mask on the bit adds two draws and one word. These
are structural diagnostics, not improvements or candidates for adoption.
Reversing the success stores ties 29 but leaves six structural rows.

After the structure-exact form, three controls preserve its complete object:
project the byte load through an unsigned arithmetic identity, cast the reset
loop's postdecrement result to unsigned, and reuse the reset pointer as the PFS
base while generating the rumble status addresses. The last form avoids the
older experiment's extra frame home but still adds no draw. All fourteen draws
and their order remain unchanged. Those three controls meet the ADR 0018 stall;
the natural three-change candidate is restored, with no diagnostic identity
expression retained.

Fresh stock align_symbol.py, score_symbol.py, register_census.py and
frame_census.py confirm 115/115 words, 0x60 on both sides and all eleven used
stack offsets equal. No new relocation-identity proof or exact C credit is
claimed. The source remains guarded NON_MATCHING. Evidence under ignored
build/g1/packInit includes every source/object pair, raw trace, Ucode map,
census comparison and aligned per-window residual. Commands include
draw_census.py, residual_map.py --object/--against, allocator_trace_receipt.py,
tools/gates.sh and finalize_plateau.py. ROM gates validate the guarded tree.

#### 2026-09-16, lane nx-b: matched and promoted, 29 -> 0 in one batch

Baseline reproduced at 29 masked, delta 0, aligned 86 exact / 29 naming /
0 immediate / 0 structural, first +0xB0, all 29 rows one ten-register ring
rotation from the pak-pattern load onward, ours one draw behind. Lane g1's
reading was right: the target spends one more ring draw before that load
and emits no instruction for it. The procedure-29 census on the incoming
source has 14 draws and none before line 1035.

The generator is L149's: `pakPattern` is a memory `u8`, so masking it again
before the bit test, `(pakPattern & 0xFF) & controllerBit`, makes ugen
draw a ring temp for an `andi` that as1 folds into the `lbu` with the draw
still spent. That one cell is 0 masked at delta 0, 115 of 115 words, frame
0x60. The five controls in the same batch: a `u8` or `u32` cast of the
pattern is a no-op and inert (29); masking the bit instead adds a word
(70, +4); assigning the unread `osPfsIsPlug` result to `ret` is inert (29);
DKR's nested `ret = osMotorInit` form with its empty `if (controllerIndex)`
is 32. The folded preheader line, the bit recurrence before the index
increment and the indexed motor-success address from lane g1 are all still
load-bearing and were not varied.

Verified with `tools/score_symbol.py` at 0/0 delta 0 before promotion and
with `gmake verify` printing the expected SHA1 from the C after it. The
NON_MATCHING guard, GLOBAL_ASM fallback and PLATEAU-HANDOFF block are
removed. Evidence is outside the tree under the lane's private evidence
directory; nothing ROM-derived is tracked.

<!-- plateau-handoff:packInit:end -->
