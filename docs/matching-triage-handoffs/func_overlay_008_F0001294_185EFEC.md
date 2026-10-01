<!-- plateau-handoff:func_overlay_008_F0001294_185EFEC:start -->
### `func_overlay_008_F0001294_185EFEC` plateau handoff

- source: `src/overlays/o008/overlay_008.c`
- score: 16 differing words
- frame: 0xB0
- relocations: 137
- first mismatch: +0xD8
- summary: Tuning read as a struct by field, or-assign flag updates, plain counter after the turn; left: v0/v1/a1 colours and one compare order.

## 2026-10-02 lane e-o008: 78 to 16, size and frame exact

align_symbol now reads 1243 byte-exact, 14 register naming, 0 immediate, 2
really different at size delta 0 (was 1184, 67, 1, 9). Every edit was a cell
of a `tools/shape_product.py` product on the whole TU; siblings unchanged
(F0000058 438, F00034A0 296, F00042A8 384).

- **The tuning block is a struct read by field** (`O8P1294Tuning`). The
  pre-loop factor is `value * 0.5f * tuning->unkC`: as a field the product
  stays on the left and the shipped draw order (0.5, product, field) falls
  out; as an array element uopt puts the load first. 78 to 70 at that site
  alone, 69 with the whole function on the struct. The previous lane's
  "struct costs 164 elsewhere" was one field: offset 0x40 must be an array
  element (`unk20[8]`), not a scalar, or the `index * unk8 + unk40` sum swaps
  its operands. A 512-cell per-site product (array element against field at
  nine site groups) found only that site and 0x10 sensitive.
- effectMask or-assigned 0x3C before `D_4 = 1;` in the reverse-stick branch (69 to
  65).
- **`owner->unk80` or-assigned the table entry** (compound assignment) at the four selector sites, and the
  selector default `angleStep = 1` assigned inside the first arm rather than
  before the `if` (65 to 43; the compound or alone is 22 of it). Or-ing the global
  back the other way round or with a cast is the old 65.
- **The frame counter needs no cast.** `state->unk182 = (state->unk182 + 1)
  & 0xF` written after `turnAmount = ...`: the trunc/mfc1 of the turn amount
  spends the ring draw the target spends before the counter load, which the
  `(u8)` cast had been supplying after it. Together with the next two edits,
  43 to 21; the cast-free counter alone is 214 (ring one behind from +0xE5C).
- **The spin-out negation reads the field**: `state->unk102 =
  -state->unk100; state->unk100 = 0;` (the load is a ring temporary in the
  target, not the drift carrier).
- **`state->unkFC += turnAmount;` before the drift step** computation.
- **The brake result borrows `angle`'s symbol** and its first two tests read
  `!(inputFlags & 0x4000)` directly (21 to 16). A fresh local for it is 40;
  `index`, `turnDirection` or keeping `braking` are 21.
- Inert, adopted as the natural spelling: `state->unk4 +=` for the motion
  scalar, `owner->unk20 += 3.0f`, `state->unk106 += 1`, the compound or of 0xC into `owner->unk80`.

Measured and not better: eleven counter spellings (casts on the load, `% 16`,
two-statement forms); the factor compare as `0.0f != value`, two-statement
or direct reads (the c.eq operand order at +0xD8 did not move, direct reads
cost 7 to 11); the 68/58 test of `state->unk100` through eight s32 carriers
(inert to +4) or through `driftDirection` (s16 sign-extension pair, +4
bytes; as s32 the frame homes move, 27); the steering copy written as a
chained assignment (106) or with the first clamp on the copy (inert); four
update-loop forms (`while (n--)` is identical code but moves a home, 23);
updateCount through ten other carriers (16 to 22, or size changes);
`state->unk349` through seven carriers or read directly (worse); four
spellings of the unkFE drift step.

What remains (14 naming, 2 structural):

- +0xD8: `c.eq.s` operand order of the factor's zero test (1 word).
- +0x20C..+0x218: the update count's first read is v1 in the target, v0
  here (3 words).
- +0xA8C/+0xA90: the 68/58 test loads `state->unk100` into a1 (the drift
  carrier's colour) after the 0x174 literal's `lui`; here it is a v0 temporary
  scheduled first (2 structural, plus the branch at +0xAA8).
- +0xB30/+0xB70/+0xB84: the second clamp compares read the copy (v0) in the
  target and the stick (a0) here: uopt copy-propagates our copy.
- +0xDBC: `state->unkFE` held in v0 in the target, a3 here (3 words).
- +0x115C: `state->unk349` carrier v0 in the target, v1 here (3 words).

Decision variables reached with the instrumented uopt (proc ordinal 7; every
force below read back `forced=` equal to the requested colour):

- **+0xDBC (3 words): web 554 against web 551.** 551 is the `state->unkFE`
  load (bbs 215-217, save 30/2 = 15). 554 is the `steeringInput >> 4` temp
  in bb 215 (save 30, nocs 1): ugen writes it to v0, copies it to `index`
  and branches on it, and as1 deletes the copy by renaming, so it never
  appears in the object. Being decided first it takes v0 and leaves 551 a3.
  `p1:w554=c6` alone gives 13 (only those three words move). Measured and
  inert: seven spellings of the `>> 4` step and its zero test (assignment in
  the condition, `!index`, if/else arms, the shift folded into the
  subtraction), eight carriers for the drift step (21 to 600, never v0 for
  551), and three forms each of the unkFC and unkFE updates.
- **+0x20C (3 words): web 114**, the trunc result of `(s32) update` in the
  loop-entry block (save 3). It takes v0; the target has v1, so something in
  the target holds v0 across that block. `p1:w114=c2` alone gives 13; with
  the force above, 10. Seven conversion spellings and ten carriers are inert.
- +0xD8: with the factor test written on the field (`0.0f != state->unk5C`
  and the field in the product) the `c.eq.s` operand order is the target's,
  but the cached load and the factor then trade f0 and f2 (27); four float
  carriers for either factor site do not trade them back (best 19).

## 2026-10-01 lane d-o008: 275 to 78, frame exact

align_symbol now reads 1184 byte-exact, 67 register naming, 1 immediate, 9
really different at size delta 0 (was 984, 271, 0, 4). Each edit was
measured as a product cell on the whole TU with `tools/shape_product.py`;
the sibling candidates are unchanged (308, 384, 438).

- **The frame-counter increment is truncated before the mask**:
  `state->unk182 = (u8) (state->unk182 + 1) & 0xF`. That spends the one
  integer ring draw the target spends there with no emitted word, and it
  re-phases the integer ring for the whole back half: 246 to 97. Without the
  cast, or with the cast outside the mask, the ring is one register behind
  from +0xCD0 to the end.
- **The stick test is a plain compare** (`(steeringInput = state->unk428) <
  -0x1E`), not a value compared with zero. The old form drew a ring register
  for the boolean where the target branches on `at`. Alone it cost 56 words
  on the old shape (the extra draw had been cancelling the missing one
  above); together with the cast it is part of the 97.
- **The pre-loop factor**: `if ((value = state->unk5C) != 0.0f)` with
  `value * tuning[3] * 0.5f`. 275 to 190 on its own; the ring it leaves is
  the target's for everything after it. The local order is still not the
  shipped one (below).
- **Carriers that the shipped registers imply.** The mathDiffAngle result
  and the drift step share one symbol (`index`), which puts both in v1 as
  shipped (97 to 94); the 0x2710/tuning turn limit is its own symbol
  (`braking`, dead by then), which takes turnAmount off the limit's web and
  puts it in a2 (94 to 84); the drift-arming test reads `state->unk428`
  through `steeringInput` at each of its two reads (84 to 79); the curve sum
  adds the base first (79 to 78).
- **Measured and flat or worse:** declaration position of `tuning` (7
  cells, 246 to 262); `tuning` as a struct pointer, all sites (339) and a
  512-cell per-site-group product (floor 245, equal to its array base); the
  drift-branch condition written five ways (inert or +56); the spin-out
  negation through four carriers or written directly (inert or +153); the
  68/58 test of `state->unk100` through any of eight carriers (inert, +4, or
  a sign-extension pair when the carrier is the s16 `driftDirection`).

What remains, by window:

- **Pre-loop factor, +0xD4..+0xF4** (5 rows including the 2 one-sided
  words). The target draws the 0.5 constant, the first product, then
  `tuning[3]`; every one-expression form loads `tuning[3]` first, because
  uopt puts the load operand first whatever the source order. Two-statement
  forms give the shipped draw order but colour the intermediate. A struct
  field for `tuning[3]` (`tuning` cast to a record) gives the shipped order
  with the product first and leaves only the compare's operand order, but
  every whole-struct form costs elsewhere. The open question is what makes
  the shipped tree keep the product on the left.
- **`state->unkFE` temporary in a3 where the target has v0** (about 8 rows
  at +0xDBC), and the integer ring one register off from +0xCD0 to +0xD7C
  (the cast supplies the draw after the frame-counter load rather than
  before it).
- `state->unk100` at the 68/58 test is a v0 temporary here and sits in a1 in
  the target; `steeringTarget` compares at +0xB30/+0xB70/+0xB84 read the
  copy (v0) in the target and the source (a0) here.
- Smaller rows: `updateCount` in v0 for v1 at +0x20C, two t8/t9 rows at
  +0x478, operand order at +0xBC4, `state->unk349` carrier at +0x115C, and
  ring rows in the colour tail from +0x11E8.

## 2026-10-01 natural-shape rewrite (lane `lane/a-o008`): 636 to 275, frame exact

The candidate was rewritten from the target's shape rather than polished. It
now has the exact size, the target's 0xB0 frame with an identical home ladder
(frame census: both sides list the same ten homes at the same offsets), zero
one-sided words and zero immediate-only rows. align_symbol reads 984
byte-exact, 271 register naming, 0 immediate, 4 really different (was 711,
475, 32, 42). The sibling candidates in the translation unit measure what they
did before (11, 39, 308, 384, 438).

What moved it, in the order it was found, each measured on the whole TU:

- **The pool floats are literals.** The forty `D_F8..D_194` externs were the
  function's own literal pool. Written as float literals at each use they emit
  one pool entry per use, in the shipped order, and the forty values were
  checked equal to the ROM's. This alone removed the only one-sided pair
  (the `&D_10` address materialisation at the loop head) and put the cached
  `state->unk4` web on the target's register. Clamp pairs are not commoned:
  `if (x < K) x = K` loads K twice, as shipped.
- **The six approved carrier merges were the wrong partition.** They do land
  0xB0, but score 657 to 671. A symbol is one web, so which phases share a
  name decides the colours. The shared carriers that agree with the target are
  noted on the declarations: clamped stick with target angle, raw stick with
  wobble term and drift target, angle step with left selector, input word with
  right selector, brake bit with reverse flag, speed level with curve index,
  `state->unk108` copy with drift step. `speed` is not a variable at all.
- **State fields are read directly.** `value = field; if (value ...)` became
  `if (state->field ...)` at thirteen sites; uopt forms the same cached load
  and the operand order of the compares then agrees. Measured singly these
  moved 0 to 2 words each and one (`scale *= D_8` in place, before the
  `unkF0` update) moved 54; together 516 to 424.
- **`scale = (state->unk148 - state->unk4) * K` written out at both sites**
  instead of through `value`. With `value` there, two single-block
  float webs take the colour scale needs (1060 against 516).
- **The pre-loop float carrier is its own symbol (`factor`).** The loop's
  `scale` web then spans 75 blocks instead of 83, its save rises from 23.2 to
  25.0 and it outranks the cached `state->unk4` web at 23.5. That is the
  f2 with f12 exchange three earlier sections of this page name; it needed no
  force. 424 to 347.
- **Operand order at the first statement** (`(f32) level * tuning[2]` before
  the add) re-phases the float ring for the pre-loop.
- Compound assignment on `state->unk4` is byte-identical to the cast form the
  page used to carry, and `*=` on the `tuning[6]` product gives the shipped
  load order.

Declaration order then solves the ladder directly (L99): updatesRemaining
fifth, turnAmount seventh, impactBoost ninth, colorEnabled tenth and eleventh,
turnDirection fourteenth, scale eighteenth, speedLimit twentieth, with the
two `s16` and the `s8`/`u8` pairs sharing a word. The target has 25 words of
locals; reading `state->unkD4` directly rather than through a `peer` local
removed the twenty-sixth and put the call-spanning temporary at its shipped
offset.

What remains, with the decision variable for each:

- **`turnAmount` holds a3 where the target holds a2**, and the temporary for
  `state->unkFE` holds a2 where the target holds v0. The drift step shares the
  `state->unk108` copy's symbol and takes v0; the target has it in v1, the
  register of the `mathDiffAngle` result. Those two sit in one block, so by
  block-level interference they are one web in the target. Writing both
  through one name was byte-inert here because the call result is forwarded
  as a temporary; the open question is which spelling keeps the call result a
  declared symbol.
- **The pre-loop `factor` product.** The target draws the constant, the first
  product, then `tuning[3]`; every single-statement form draws `tuning[3]`
  first. A three-statement form through a second float local draws in the
  target's order but colours the second product. About forty forms measured;
  the free order this leaves behind shifts the float ring for the pre-loop.
- `state->unk428` at the drift-arming test and `state->unk100` at the 68/58
  choice are temporaries in v0 here and sit in the stick and drift registers
  in the target; routing them through those symbols changes size by one word
  in each direction tried.
- Four schedule rows: the `lh`/`lui` order at +0xA8C and the load order of
  `state->unk102` against the selector constant at +0x10F4.

The earlier force landscapes on this page were measured on the extern-pool
shape and do not describe this candidate.

## 2026-09-12 exhaustive colour landscape

`residual_map.py` re-derived 711 aligned exact words and 547 aligned residual
rows (475 naming, 32 immediate, 40 structural). The sole target-only/candidate-only
bracket is `+0x224 -> +0x468`; it accounts for 88 aligned-exact rows that are
position-only shadow, so its 636 positional score must not be read as 636 colour rows.

All 910 legal single forces were measured at zero size delta. The 21 winners
were `w149=c26(614)`, `w625=c6(626)`, `w627=c6(626)`, `w629=c6(626)`,
`w325=c3(627)`, `w335=c3/c4/c5/c6(627)`, `w406=c26(628)`,
`w325=c4/c5/c6(629)`, `w239=c3(631)`, `w124=c2(633)`, `w486=c5(633)`,
`w68=c2(635)`, `w406=c27(635)`, `w431=c4(635)`, and `w523=c4/c5(635)`.
The packing selected `w149=c26,w625=c6,w239=c3,w124=c2,w68=c2,w406=c27,w523=c4`:
predicted 593 and measured 565. The 28-word synergy between the two float
forces is real; the measured colour floor is therefore 71 of 636 positional
words (and at most 71 of 547 aligned rows), leaving 565 positional words outside
this one-force colour landscape. No source form was adopted.

## 2026-09-11 phase census and force-sweep verdict (lane `lane/p9-alloc`)

No source change is adopted. Three measurements that narrow what the naming
bucket actually is.

**Phase census.** The instrumented IDO 5.3 uopt -- `.text` first checked
byte-identical to the tree's own object for this TU -- emits **124 p1 decisions
for this procedure (55 `color`, 69 `split`) and zero p2 records.** L106 has no
axis here, so web numbering, definition position, declaration order and
statement order are all dead ends; only L100's ratio decides anything p1
decides.

**The naming rows are mixed, and that matters for routing.** Splitting them by
register bank: **197 name only floating-point registers, 263 name only integer
registers, and 15 name both.** Two different mechanisms are therefore active
at once, and an edit aimed at one will not move the other. The integer side is
a four-step ring rotation -- t5 to t6, t4 to t5, t7 to t4, t6 to t7 at 25, 22,
20 and 17 slots -- which is a ugen temp-ring phase (L41, L64), not a colour.
The floating-point side is two *pairwise* exchanges rather than a rotation:
`$f12` with `$f2` at 56 and 55 slots, and `$f16` with `$f10` at 41 and 41,
both starting in the first 0x300 bytes.

**Force-sweep verdict.** Forcing every colour each web's own `p1cost` record
declares available, each force confirmed accepted in the records, and then
searching greedily three deep, moves the count from 636 to **612** -- naming
475 to 451, structural unchanged at 74, size delta 0. So **p1 colouring
reaches about 24 of the 475 naming rows.** The other 451 are ring phase, and
the ring is set by the order in which expression temporaries are allocated and
freed, which is an expression-shape and operand-order question at the earliest
differing site, not an allocator-cost question anywhere.

## 2026-09-10 region partition (lane `lane/w8-recon`): the residual is naming, not structure

No source change is adopted here. The pass replaces the standing description of
this residual ("24 excess frame bytes and 18 opcode edits") with a measured
partition, and closes one axis with a complete lattice.

### The residual, split by cause

Aligning the two instruction sequences on a register-erased shape --- opcode,
function code, shift amount, immediate and branch offset kept, every GPR *and*
FPR selector erased --- and classifying each aligned pair:

- function: 1259
- positional differing (masked): 636
- byte-exact once displacement is removed: 727
- **displacement tax**: **104 (16% of the residual)**
- **register-naming-only rows**: **489 (77%)**
- rows differing in instruction kind, or unalignable: 55 (9%)

So this whale is *not* a displacement problem and never was: fewer than a fifth
of its residual is instructions sitting in the wrong place, and 55 words is the
whole of what is actually wrong. Displacement is confined to one clean -1 run
over +0x228..+0x46C; everywhere else the two sides are in phase.

### The dominant term is two float-register transpositions

Of the 489 register-naming rows, **209 differ only in a floating-point register
selector**, and they are dominated by two transpositions that run the length of
the function and go in both directions:

- `$f2` exchanged with `$f12`: 117 sites
- `$f10` exchanged with `$f16`: 89 sites
- everything else (`$f4`/`$f6`, `$f18`/`$f0`, `$f8`/`$f6`): about 60 sites

Two float webs hold each other's colours, and a second pair does the same. The
integer side is 267 rows and is a position-dependent rotation of `t0`-`t9`
(+1 through the middle of the function, reversing near the tail) plus a
`v0`/`v1`/`a0`/`a1` shuffle in the entry region. **The callee-saved integer
allocation is already exact: there are zero `s`-register substitutions anywhere
in the function.** No prior pass recorded that, and it removes a whole family of
hypotheses.

This is the first lever to attack. It is one allocation decision replicated
across a third of the function; the earlier "scoped allocation/alias analysis"
route was right about the axis but was aimed at the integer side.

### The 24 excess frame bytes are six words of declared local, and declaration order is not the axis

A typed stack-home census settles what the frame difference is and is not.
Every sp-relative reference on both sides is a plain load or store --- neither
side forms a stack address in a register --- so the census is complete:

- **19 distinct homes and 50 accesses on each side.** The outgoing-argument
  slots, the two double saves, the four saved-register slots and the return
  address are at *identical* offsets on both sides.
- The nine non-save homes correspond one-for-one, with matching access counts
  and matching widths: two float locals (2 accesses each), one 5-access int,
  three 3-access ints, two 2-access ints, and the 2-access int at +0x44 which
  is at the *same* offset on both sides.
- So the candidate has neither an extra local nor a missing one. The +24 bytes
  are 6 words of *declared* storage the target function did not have.

The frame responds to declared local bytes and nothing else here: growing the
`colorEnabled` array to 4 and 6 elements moves the frame 0xC8 -> 0xD0 -> 0xD8
in 8-byte steps while leaving the emitted code identical (636 masked, 727
aligned-exact). Reaching the target's 0xB0 therefore needs the candidate's
declared locals to shrink by exactly 24 bytes.

**Declaration order does not do it.** A complete move-one lattice over all 32
declarations --- 962 compiled and scored candidates, every ordering reachable by
relocating one declaration --- leaves the frame at 0xC8 in *every* case. The set
of frames reached is `{0xC8}`. The best score in the whole lattice is 630
(`colorEnabled` moved to position 16) against a base of 636, and the
aligned-exact count moves 727 -> 733; nothing there is worth adopting. The
standing instruction to "recover the stack-home layout before any further
spelling search" therefore cannot be followed by permuting declarations: the
frame is decided by how many locals exist, not by their order.

None of the 32 locals is dead (each is read and written), so shedding 6 words
means merging six pairs of disjoint-lifetime carriers. That is a bounded search
with a hard stopping criterion --- the frame reaching 0xB0 --- and it is the
second lever.

### Next levers, in order

1. The `$f2`/`$f12` and `$f10`/`$f16` float-web exchanges (209 rows).
2. Six carrier merges to bring the declared local bytes down by 24 (frame
   0xC8 -> 0xB0); the home *set* is already correct, only the count is not.
3. The 55 kind-differing words, which is all that is left of the structure.

Do not re-run the declaration-order family: 962 measured candidates, frame
flat, score flat within 6 words.

Validation: measurements come from `tools/score_symbol.py`'s comparator; the
lattice harness reproduces its baseline exactly (636 masked, first +0x0, frame
0xC8, size delta 0). `gmake verify` passed with the expected US ROM SHA1. The
tracked source is unchanged by this pass.

2026-09-08 decompiler-assisted reconstruction, assignment base `0d21245897f0469ec630a49a07024b5e51ad04b4`:

- The lane independently reproduced the configured full-TU baseline: 1,259 words, 1,129 raw and 1,127 relocation-masked differences, first mismatch `+0x0`, frame `0x180`. The workbench verdict was `structure-mismatch`; its `next:` route was constant-audit before structure and register classes. The structure-buckets and stack-frame-recovery guides were also consulted. The prior donor and flag results below were retained; no donor body, flag lattice, permutation, compiler modification or instruction patch was used in this packet.
- Runtime metadata re-established all 137 records and 63 target identities, including the 15 calls. The local helper overwrites its incoming fourth argument before use, confirming the existing three-argument call. Names in the raw extracted assembly alone are insufficient because every call initially shares a placeholder.
- Fresh m2c generation was attempted after `./generate_ctx.sh`. The aggregate project context failed on an unrelated flexible-array field and then an incomplete SDK structure. An ignored minimal context with the authenticated call prototypes produced the draft. Drafts, reconstructed relocation annotations, objects and diagnostic rows remain only in ignored `m2cfiles/` and `build/wb/o008-reconstruction/`. No generated draft text was copied into tracked source or comments.
- Three semantic defects in the inherited candidate were corrected using Mickey's own instruction/dataflow evidence: the loop now tests the previous count while storing the next count, so an update of one executes one body; both braking-global writes are halfwords rather than one halfword and one erroneous word; the angle-difference helper receives the complete signed 32-bit requested angle. The resident callee subtracts its full arguments and performs one wrap adjustment, so caller-side halfword truncation was not established by its behavior. The local declaration and call now preserve that ABI.
- Float tuning and interpolation accesses now use float pointers. Sequential value/scale phases retain explicit loads while sharing nonoverlapping carriers. The signed-halfword steering fields use a promoted integer carrier for arithmetic and retain explicit narrowing at stores. Paired color-enable values use a two-element array, recovering the observed memory-backed channel flags. Raw steering input and its clamped value stay separate; collapsing them had removed target control/dataflow. Locals now describe these roles instead of draft register names.
- Attempt 25 was the last strict best: 1,259 words / 5,036 bytes, 636 masked and 654 raw differences, first `+0x0`, frame `0xC8` against `0xB0`. This is 491 fewer masked differences and 184 fewer frame bytes than the independently reproduced baseline. Opcode alignment has 18 edit operations (4 replacements, 7 insertions, 7 deletions); this diagnostic is not an exact-match score. The configured static surface has 137 records, 102 offset/type alignments and 88 authenticated identity alignments against 137 runtime records. Identity resolution remains partial; none of these counts establishes object or linked equality.
- Source/ABI improvements were banked throughout the reconstruction artifacts. Notable intermediate measurements were attempt 11's counted loop at 1,256 words / 1,013 masked differences; attempt 16's corrected angle ABI at 1,256 / 1,013; attempt 21's explicit final Boolean alternative at 1,258 / 932; and attempt 25's distinct steering carriers at the exact size / 636. The old exact-size baseline is semantically defective and is not the candidate to restore merely because its size already agreed.
- Stall evidence: after attempt 25, attempts 26 and 27 captured the velocity-history value with shared and separate carriers (both 1,264 words, 1,128 and 1,129 masked differences); attempt 28 moved the history write before disjoint impulse stores (1,262 / 1,092); attempt 29 removed the redundant named sound-address carrier (owned bytes and relocation tuples unchanged); attempt 30 reused the expired integer index for the cooldown load (1,259 / 644); attempt 31 represented all top-speed accesses through one pointer (1,267 / 1,212, and 146 relocation records). These six attempts produced no better retained residual, new target identity or elimination of the remaining carrier/aliasing hypothesis. They are failed source forms, not a proof that allocation is unreachable. Earlier rejected forms include nested phase scopes, a value-position compound Boolean predicate, and a single-use top-speed pointer; their artifacts and measurements are retained.
- The final source retains attempt 29's equivalent cleanup and readable local names. A direct extraction from the configured full-TU object confirmed that the owned 5,036 bytes and all 137 static relocation tuples are unchanged from that retained result. The assembly fallback remains canonical. No bytes are credited and no candidate linked-ROM equality is claimed.
- Next concrete lever: request a specifically scoped causal allocation/aliasing packet on this new source/handoff pin. Start with the 24 excess frame bytes and the different scalar/field-cache webs around impulse history; the remaining opcode edits also include the top-speed address materialization before the local helper. Recover the stack-home layout and register webs with compiler evidence before any further spelling search. Generic permutation and flags remain outside this reconstruction authorization. Exact promotion will still require complete relocation identity/offset/type proof, canonical linked-range extraction, and a passing full-ROM verification.

Validation for the retained reconstruction:

- `tools/finalize_plateau.py` refreshed the guarded source marker and structured handoff with the measured frame, score, relocation count and first mismatch.
- `gmake -j$(sysctl -n hw.ncpu) verify` passed: the canonical ROM SHA-1 is `507341c0a40ca3e9a7cee969b396ee53facfb548`. Independent extraction of the canonical assembly-fallback range `0x185EFEC..0x1860398` also compared equal to the baserom. This validates preservation of the fallback, not promotion of the candidate C.
- `gmake check-nonmatching-builds` passed for all 243 candidate-bearing translation units. `gmake cleanroom`, `gmake check-docs` and `git diff --check` passed. The documentation gate rechecked atlas/alias projections, derived numbers, the pinned donor ledger and current structured handoffs.
- The committed change is limited to `src/overlays/o008/overlay_008.c` and this symbol's handoff. Source snapshots, configured objects, comparison summaries, first-mismatch evidence, hypotheses and rejected forms are retained in ignored `build/wb/o008-reconstruction/attempt-0` through `attempt-31`, with final evidence under `final/`; fresh drafts remain in ignored `m2cfiles/`.

Historical committed evidence from before this packet (measurements below describe those earlier candidates):

Mechanism-driven evidence from the original reconstruction base `4b4109b583ca0a37427e0f38f3fa0fbb14474220` and the size-near reopen base `dd11f1cf18fa33146780764dddbb2e0d5db379c8`:

- The authenticated overlay-8 text range is `+0x1294..+0x2640` (5,036 bytes, 1,259 words). Its only in-tree caller is the same overlay at `+0x6A4`; the call and preceding C establish the three-argument `void (O8P0058Owner *, O8P0058State *, f32)` ABI.
- The new `NON_MATCHING` body is a typed reconstruction of the complete control-flow draft, including the per-update loop, motion/angle integration, resource lifetime, selector flags, and color propagation. It compiles with the TU's configured `-O2 -mips2 -Wab,-r4300_mul` policy; the ordinary build continues to select the authenticated assembly fallback.
- All 137 shipped runtime records resolve to 63 concrete identities. The 15 calls are `overlay8GetIndexed`, local `func_overlay_008_F0001000_185ED58`, three `controlSetRumble`, `func_800299E8`, three `func_800031E8`, three `func_80002FE0`, `func_8002A204`, `mathDiffAngle`, and `func_800031C0`; the local `+0x1000` helper and all rumble calls take three arguments, not the former four-argument proxy forms.
- The former `D_0` source alias conflated three target identities: the motion scalar at reserved module `4095:+0x458C4`, impact gate at `4093:+0x31A4`, and final color gate at `4093:+0x31AC`. They now have separate typed roles. The 40 `D_F8..D_194` fields remain distinct lexical identities but are authenticated as one local target block at `+0x74A8..+0x7544`; expressing them through one C base is rejected because IDO commoned the address and removed 33 target instructions.
- Configured V0 now emits the exact 1,259-word target size in a `0x180` frame versus the target's `0xB0`. Retail's `temp_v0+0x14` path is explicitly `lwc1`, `trunc.w.s`, `mfc1`, and the hazard `nop`; the former integer proxy collapsed those four operations to one. Reusing the already-declared, non-overlapping `var_f0` carrier improves the direct-float form from 1,128 to 1,127 masked differences and reduces alignment gaps from 244 to 48. Raw differences are 1,129, first mismatch is `+0x0`, and the verdict remains `structure-mismatch`.
- The earlier strict gains were: authenticated callees/arity/return types (`1264 -> 1261` words, `0x1B8 -> 0x1B0`, `1223 -> 1211` differences); typed entry parameters (`0x1B0 -> 0x1A8`); direct resource fields (`0x1A8 -> 0x190`); and changing the capped `state+0x192` carrier from `u8` to `s32` (`1211 -> 1030` differences, 798 opcode mismatches). Removing four inert m2c homes reached the `0x180` frame without changing code bytes. The old 1,256-word/1,030-difference basin is retained only as an ignored diagnostic because its `s32` field read contradicts the target operations.
- The full 119-combination flag lattice found no exact result. Removing the TU-specific assembler scheduling flag reached exact size but still left 1,201 differences from `+0x0`; that flag is shared by the translation unit and is not a source-authentic promotion lever.
- Pinned DKR v77 `src/racer.c` supplies a permitted semantic skeleton for the top-speed, spin/drift, steering, sound, and particle phases. It confirms the absolute-speed cap/subtract/floor curve, drift multiplier 68 versus 58, reverse steering sign, signed spin state, rumble, and `+/-0x2EE` yaw clamp; JFG and exact-object donor scans remain negative.
- The linked promotion trial has no measured in-range or collateral differences because it stops on a 24-byte overlay text-size delta; it is not evidence of equality. Resume from the exact-size function by removing automatic homes without replacing explicit single-load carriers with repeated field reads: a prior 13-copy direct-access batch reached a `0x158` frame but regressed structurally. The target has nine non-save homes at `+0x44/+0x60/+0x68/+0x78/+0x84/+0x88/+0x8C/+0x94/+0x9C`; candidate still has `0xD0` excess frame. Do not run flags or generic permutation until the home set is structurally close.

## 2026-09-11 closure re-test (lane `lane/f10-fell`): the f2/f12 exchange was never in the sweep

No candidate change. Records from the instrumented uopt, `.text`
byte-identical to the tree's object, procedure ordinal 7 of the TU.

The f12 web is 51 (class 2, save 480/22 = 21.8, home -36 from the frame
top). Its `p1cost` list is c26 c27 c28 c29 at 10.0 and the callee-saved
colours at 60.0; its forbidden mask is 0xc0, which is f0 and f2. f2 is held
by web 149 (save 400/16 = 25.0) and web 406 (save 190/7 = 27.1), both decided
before 51. The force sweep above forced only colours each list declared
available, so the exchange this page names as its first lever was never
forced. Forcing 149 to f12 and 406 to f14 together, with 51 unforced, gives
51 f2 and takes 636 to 606; the aligned naming rows that name only a float
register go 197 to 156 while the integer ring re-phases behind it. The other
pairing, 149 f14 and 406 f12, measures 625.

The decision variable is the p1 rank of web 51 against 149 and 406, which is
L100 arithmetic: 149 falls below 51 at nocs 19 or with 51 fewer points of
net, and neither is a declaration or placement axis. The 24-row bound above
is a bound on the swept colours, not on this exchange.

## 2026-09-11 frame arithmetic closed, and one inherited description corrected (lane `lane/o11-frames`)

No source change adopted.  Two declaration reorders aimed at the target's home
ladder were built and measured byte-identical to the current candidate (636
masked, 711 aligned byte-exact, 475 naming, 32 immediate-only, 42
really-different, delta 0), which is consistent with the 962-candidate move-one
lattice already on this page.

**The correction.**  A dispatch reached this lane describing the frame gap as
"one escaping pointer homed at +0x54 instead of +0x40, under three float homes,
with 32 of 36 sp rows being the resulting constant -4".  That does not describe
these two objects.  Neither side touches +0x40 or +0x54 at all.  The slots each
side actually uses, in full:

- both: 0x10, 0x14, 0x18, 0x20, 0x2C, 0x30, 0x34, 0x38, 0x3C, 0x44
- this candidate only: 0x6C, 0xA4, 0xA8, 0xB4, 0xB8, 0xBC, 0xC4, 0xD0
- the target only: 0x60, 0x68, 0x84, 0x88, 0x8C, 0x94, 0x9C, 0xB8

Whoever writes that description next should read it off the objects.

**The arithmetic, which the page already had qualitatively.**  Every slot below
0x4C -- the eight argument words, the two saved doubles at 0x20 and 0x28, the
four saved registers and `ra` at 0x2C..0x3C, and the two-access compiler
temporary at 0x44 -- sits at an identical absolute offset on both sides.  So the
frame difference is block and only block: **124 bytes here, 100 in the target,
exactly six four-byte cells**, which confirms the earlier "6 words of declared
storage" with the slots rather than by subtraction.

**Where the target's eight homes sit,** measured from its own frame top, with
this candidate's for comparison:

- target: -20 (3 loads 2 stores), -28 (1/1), -36, -40, -44 (three 1 load 2
  stores), -56 (2/1), -72 (1/1), -80 (1/1)
- candidate: -4 `updatesRemaining` (3/2), -12 `impactBoost` (1/2),
  -16 and -20 `colorEnabled[1]`, `colorEnabled[0]` (1/2 each), -32 `speedLimit`
  (1/1), -36 `scale` (1/1), -80 `turnAmount` (1/2), -92 `turnDirection` (1/1)

Matching on traffic: `updatesRemaining` is the target's **fifth** declaration,
not its first -- four cells precede it.  `impactBoost` is ninth and
`colorEnabled` tenth and eleventh, so three cells sit between `updatesRemaining`
and `impactBoost` where this candidate has one.  That is a constraint the six
carrier merges have to satisfy jointly, and it is the thing to solve against
rather than searching merges by score.
<!-- plateau-handoff:func_overlay_008_F0001294_185EFEC:end -->
