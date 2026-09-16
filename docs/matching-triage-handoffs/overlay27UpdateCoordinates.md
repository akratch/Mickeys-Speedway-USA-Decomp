<!-- plateau-handoff:overlay27UpdateCoordinates:start -->
### `overlay27UpdateCoordinates` plateau handoff

- source: `src/overlays/o027/overlay_027.c`
- score: 0/65 words, promoted
- frame: frameless
- relocations: 10
- first mismatch: none
- summary: Matched 2026-09-16 (lane s1-a); the countdown loop is a `while (remaining--)` whose condition temp is numbered ahead of the loop's index expressions.

#### 2026-09-13, lane g1: preheader and carrier draw controls

The assignment gate returns base-only. A retained Ucode capture and
allocator_trace_receipt.py map this symbol to procedure 3 of six. The configured
stock and traced full-TU text agree. Baseline: 260 bytes, zero size delta,
46 aligned exact words and 19 naming rows, first +0x0, frameless, ten relocations.
The old score header's 19 is the differing count, not the exact count.
The procedure spends 29 draws and 118 emission events. No colour sweep is rerun.

Four separately preserved source controls test the preheader and loop:

- Reversing the X assignment chain removes one draw on that line, while total
  emissions remain 118. It adds one executable word and regresses to 51 naming
  plus three structural rows, with a candidate-only offset at +0x38.
- Replacing the declared record cursor with an indexed array access preserves
  all 29 draws and their order, but adds 19 emission events. Size remains equal;
  aligned residual becomes 26 naming and 11 structural rows, with four offsets
  unique to each side. The generated-cursor form does not retain the loop shape.
- Unsigned countdown type changes no line's draws or emissions and produces
  the same 19-row object.
- Swapping the countdown and Y-offset carriers also leaves every draw and
  emission count and the draw order unchanged, but grows the aligned naming
  residual to 33. This reaches colouring without changing the draw schedule.

The original 19-row body is restored. None of these controls improves the best
residual or opens a source lever beyond the existing web/address-order blocker;
the latest unchanged-schedule controls and recorded grouped-store regression
support stopping rather than repeating a colour or order sweep. The next
question remains the countdown/address web creation and table-base completion
order, requiring a source form that preserves the existing loop. This is a
NON_MATCHING plateau and carries no new matching credit. Source snapshots,
objects, raw traces and aligned comparisons remain ignored under
build/g1/overlay27UpdateCoordinates. Commands: draw_census.py,
residual_map.py --object/--against, register_census.py, allocator_trace_receipt.py,
and finalize_plateau.py.

#### 2026-09-16, lane s1-a: matched and promoted, 19 to 0 -- the countdown temp's web number

Baseline reproduced at 19 masked, delta zero, first +0x0, aligned 46 exact
and 19 naming, no structural rows. One cycle, thirteen cells. Verified:
`gmake verify` prints the expected SHA1 from the C with the pragma gone,
`gmake promotion-proof` passes (65 words, frameless, 10 of 10 relocation
identities, identity static), `gmake check-overlay-syms` up to date.

The function is a leaf, so the call test says p2 only: colours are assigned
in ascending web number, lowest free colour (L141, L154). The lineage
census on procedure 3 numbers the declared symbols 0, 7, 16 and 19, the
loop's three scaled-index expression webs 23, 33 and 43, the post-decrement
temp of `remaining--` 54, and the address constants 61 to 67. Read against
the listing that order gives ours exactly: countdown a1, then the indices
a2/a3/t0 and the temp t1, with the X-offset address on a2. The ROM has the
temp on a1 ahead of the indices (a3/t0/t1) and the countdown on a2, which
needs the temp numbered before the index expressions. Web numbers follow
first-definition order in the body (L106), and a `do { } while (remaining--)`
creates the temp after the body. `remaining = 10; while (remaining--) { }`
creates it in the condition, before the body: uopt propagates the constant
through the peeled first test, so the shape is the identical do-while with
the preheader 9, and the numbering is the ROM's. Ten iterations either way.

Controls in the same batch: `while (remaining-- != 0)` is a different object
(16 at -12 bytes, the truth test folds differently from the comparison);
`for` counting up or down and a pre-decrement `do` all change size by 104
to 108 bytes; six declaration orders and an explicit copy temp are inert at
19; defining `remaining` before `record` is 28 and before the offsets 33
(first-definition order moves it the wrong way); a pointer local for the
X-offset global is 59; an indexed-record loop 242 at +708; a pointer-bound
loop 61 at -12. The prior shard's "next question" (countdown/address web
creation order) was the right variable; the lever was the loop keyword, not
the initialisers or carriers it had swept.
<!-- plateau-handoff:overlay27UpdateCoordinates:end -->
