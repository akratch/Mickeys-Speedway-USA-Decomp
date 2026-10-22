<!-- plateau-handoff:overlay98CheckObject:start -->
### `overlay98CheckObject` plateau handoff

- source: `src/overlays/o098/overlay_098.c`
- score: 0/111 words
- frame: 0xA8
- relocations: 6
- first mismatch: none
- summary: ROM-exact 444-byte promotion; named pointer and float intermediates, delayed acceptance initialization and restored stack homes.

Fresh lane measurement (2026-09-11, lane/p7-mix): moving `s32 scratch[10]`
before the vector/local declarations is a strict configured-TU gain, 35 to 31
differing words, with exact stack-home ladder. The remaining 29 naming sites
retain one coherent `s7 <-> fp` cycle plus the float-bank shift. A bounded
20-minute permutation reached scratch score 165 only with an inert
`if (result) {}` guard; that diagnostic was rejected and no exact result was
promoted.
#### 2026-09-12, lane p23-lastmile5: exact C and full ROM proof

The fresh baseline was 111 words, 31 raw and masked differences, delta zero:
80 exact, 29 naming, zero immediate and two structural rows. No insertion or
missing instruction was present. Stock/instrumented fidelity passed all five
section, symbol and relocation gates. This single-function TU uses procedure
zero. The exhaustive same-save-kind landscape covered all 13 coloured webs,
with 60 compiled alternatives and one web with no alternative: 55 accepted at
delta zero and five accepted with larger geometry. The complete winners list
is `p1:w31=c22` at 22, `p1:w86=c22` at 29 and `p1:w87=c22` at 27. The packing
selects only web 31. All seven nonempty subsets were tested; every multi-force
combination was refused because at least one requested colour did not apply.
The diagnostic lattice floor was 22, not a source lower bound.

The source closes four independently measured requirements. Naming the
node-data pointer corrects the integer temporary consumption. Moving the
acceptance-zero assignment after the height-selection branch corrects the
saved-register exchange. Naming the inner floating subtraction supplies the
second allocated float carrier instead of a temporary. Declaring both added
carriers before the unused scratch array and reducing that array to eight
words restores the original vector homes and the 0xA8 frame. The scratch
contents are never read or written; its extent is not gameplay data. The
calculation's evaluation and call order, comparison behavior, and return value
are preserved. No new inert expression or forced compiler output is shipped.

Measured milestones: named data 21; delayed initialization alone 22; both 12;
frame adjustment 10; data home moved before scratch 6; named floating
difference with the restored home layout 0. Final aligned buckets are
111 exact and zero naming, immediate, structural, or unmatched rows. The
untouched stock output has zero raw and masked differences and six exact
relocation sites. All intermediate source/object pairs remain private under
`build/p23/overlay98CheckObject/`.

Promotion completes this standalone TU; no mixed-TU range addition is needed.
The atlas and its donor digest were regenerated, extraction preceded the first
alias pass, and a second alias generation and rebuild followed the first build.
`gmake promotion-proof SYMBOL=overlay98CheckObject` passes: 111 words,
frame 0xA8, relocations 6/6, static identities exact. The linked owned text
range is overlay 98 +0x848 through +0xA04, 444 executable bytes; the following
12 bytes of padding remain separate and earn no credit. Full `gmake verify`
through the gate runner reproduces the expected US ROM hash. No post-compile
instruction edits were made. There is no remaining function deficit.

<!-- plateau-handoff:overlay98CheckObject:end -->
