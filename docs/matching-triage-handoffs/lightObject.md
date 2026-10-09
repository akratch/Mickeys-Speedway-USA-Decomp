<!-- plateau-handoff:lightObject:start -->
### `lightObject` plateau handoff

- source: `src/main/lights.c`
- score: 0/184 words, promoted
- frame: 0xC8
- relocations: 28
- first mismatch: none
- summary: Matched. Scale read from the field at both uses, factor first, and the scaled bytes written back so blue16 and packed10 are computed from the stored fields (forwarded ring temps).

Summary before this remeasure: 63 to 56: factor load between the green and blue saves moves the high-half draw two ring slots. Left: ring phase t4 vs t6 and the changed constant draw

Summary before this remeasure: hypothesis=JFG efd5abb lightObject C with Mickey types; spellings=none, src/lights.c is still GLOBAL_ASM; stall=kept 63 words at delta 0

Summary before this remeasure: Sunk partially dead lightData def closes the -4; target homes give 63 at delta 0; three single-block CSE temps take v0/v1/a0 not ring temps

Summary before this remeasure: 183/184 words; early scaling-pointer hoist swaps the s1/s2 carrier; flag lattice lacks a resident size owner

#### 2026-09-23 (Track B, lane B-resmix): delta 0, 155 to 63

Cycle 0, `tools/insertion_pairs.py`: three pairs. Pairs 1 and 2 (missing-CSE
and hoist) were owned by the scale-test line and the `description + 0x10`
lines: the target computes that address inside the scaled branch and again
at the join, and addresses the branch's byte fields through it, where the
candidate computed it once above the test and folded the 0x10 into every
offset. Pair 3 was the tail shadow.

Measured, one at a time:

- merging `scaledData` into `lightData`, or re-spelling either address site
  (`description->pad10`, an integer add, an unsigned literal): flat at 155.
  The spelling was never the question; uopt hoists the one expression.
- assigning `lightData` once before the scale test and again after it: 75 at
  delta 0. The first definition is dead on the unscaled path, so uopt sinks
  it into the branch; that is the target's shape exactly. The position of
  that first assignment among the three statements before the test is
  inert (75 each).
- declarations reordered to the target's homes (count, the four saved
  values and changed at the top, `local` at 0x70, `cameraDelta` at 0x50,
  searched over every split of the other seven): 63, slot ladder identical,
  immediate bucket 0.
- moving `changed = 1` through all 12 positions in the branch: 63 to 73,
  none better.
- removing the redValue and greenValue carriers (three forms): flat at 63;
  uopt had already propagated them.

Remaining, 57 naming rows and one moved word (candidate +0xF8, target
+0xD0). The records on this shape show three single-block expression webs
of the scaling arithmetic (78, 86, 88; type 6, bb 13, save 3 each) coloured
`v0`, `v1` and `a0` by globalcolor, where the target keeps those values in
ring temps; from there the ring phase differs (t0-t4 here against t5-t9 in
the target). The constant for `changed = 1` is drawn later than the target
draws it. The next question is what makes those three temps ineligible for
globalcolor in the target's source, not their colour.

2026-09-26: JFG public src/lights.c at efd5abb still wraps lightObject in
GLOBAL_ASM. The TU advance past the older reference is symbol renaming and
lightDistanceCalc dropping its NON_EQUIVALENT guard, not a C body for this
function. The public symbol span is the same 736 bytes. Zero spellings of a
port. Remeasured the kept body at 63 masked words, size delta 0, frame 0xC8
on both sides, 28 relocations with the same types and offsets, first
mismatch +0x80.

#### 2026-10-01, lane d-res2: 63 to 56, register-only

The first mismatch after the earlier work was the high-half draw of
`D_8007C85C`: the target has it in t4, right after the two test draws
(t2, t3); the candidate drew it two ring positions later because the three
opening saves (red, green, blue) were drawn first. Measured as products at
delta zero, frame 0xC8:

- position of the factor line among the five opening statements and of
  `changed = 1` (5 x 7 cells): best 56 with the factor between the green and
  blue saves and `changed = 1` last; factor first 58, factor after the first
  save 61, original 63; `changed = 1` anywhere earlier 63 to 73.
- all 120 orders of the five opening statements: 56 only for red, green,
  factor, blue, packed; the next best are 58 (four orders).
- all 420 admissible orders of the seven statements that follow (red value,
  red store, green value, blue store, packed store, green store, `changed`):
  every one scores 56, so that block is inert.
- all 720 orders of the five opening statements plus an early `changed = 1`:
  63 at best.
- `(scale != 1.0f)` for the swapped `c.eq.s` operands: inert.

The target's text order (red, green, blue, packed loads) with the factor draw
before all of them is not reachable by any statement order: the ring phase
needs the factor draw before the saves and the saves in source order, and the
two together score 58. The decision variable left is the draw count between
the test and the scale block (two draws: t5 and t6 in the target's ring).
## 2026-10-02 (lane e-res3): no change, 56 held

Product over the scale-test spelling (`1.0f != scale`, `scale != 1.0f`,
`!(1.0f == scale)`) and the factor expression (`scale * (a * b)`, `a * b *
scale`, `(a * scale) * b`, `scale * (b * a)`): the first two factor forms and all
three tests are 56; `(a * scale) * b` is 63. The `c.eq.s` operand order and the
`mul.s` operand order do not move with the source spelling. Decision variable
unchanged: the two ring draws (t5, t6) between the test and the scale block.

## 2026-10-02 (lane x-res): 56 to 0, promoted

Matched by rewriting the scale block, not by any colour or order work.
Three facts, each measured as a product with `tools/shape_product.py`:

- The `scale` local was a carrier (checklist item 1). Reading
  `description->scale0` at both uses flips the `c.eq.s` operand order and the
  `mul.s` operand order to the target's; with the local both were swapped
  whatever the spelling (the e-res3 product varied the spelling only, inside
  the local's shape).
- The redValue/greenValue locals were the three globally coloured webs. The
  target writes the scaled bytes back and derives blue16 from the two STORED
  fields, then packed10 from blue16: `red15 = (s32)(savedRed * factor);
  green17 = (s32)(savedGreen * factor); blue16 = red15 - green17;
  packed10 = blue16 << shift14`. uopt forwards each stored value, so the two
  products and their difference stay ring temporaries (t5, t6, t7) and the
  forwarded u8 is the target's `andi` before the `sllv`. Writing the
  difference twice instead of reading blue16 back makes it a CSE web (v0)
  and costs one ring draw, which as1 then repays by hoisting the
  D_800794A0 high half into the c.eq.s hazard slot (size delta -4).
  Without the casts to s32 the float-to-u8 conversions are +240.
- With those two in place the factor goes first in the scaled block (its
  D_8007C85C draw follows the two test draws: t4), the d-res2 closure's
  "factor first is 58" was true only of the carrier shape.

The frame needs three declarations in the slots the removed locals held
(between lightData and cameraDelta): any two of the three leave 33 at delta
0, none 37. They are declared and unused.

Closure broken: e-res3's "c.eq.s and mul.s operand order do not move with
the source spelling" holds only while `scale` is a local; d-res2's "the
target's text order with the factor draw first is not reachable by any
statement order" holds only on the redValue/greenValue carrier shape.

Gates: gmake verify OK on the promoted tree, scoreboard regenerated.

<!-- plateau-handoff:lightObject:end -->
