<!-- plateau-handoff:func_overlay_052_F000063C_189ACAC:start -->
### `func_overlay_052_F000063C_189ACAC` plateau handoff

- source: `src/overlays/o052/overlay52TailB.c`
- score: 0/1687 words, promoted
- frame: 0x118
- relocations: 314
- first mismatch: none
- summary: Matched. -Wab,-r4300_mul; overlay data defined in the TU and dropped at link; the shipped set of locals (lap and alpha own, time difference in iconX's, first and second HUD rows indexed); (u8) alpha; shipped evaluation orders.

Summary before this remeasure: Own lap/alpha locals, HUD rows indexed, difference loaded then negated: 19 to 2. Open: player*48 spill cell 0x64 not 0x60.

Summary before this remeasure: Address-order ring fixes: late HUD pointer, compare/rounding order, (u8) alpha: 500 to 19. Open: lap s1 not v0 (iconX web).

Summary before this remeasure: Lap and alpha in i, mode bit and buttons one local: 506 to 500. Open: lap colours t1 not v0; temp-ring rotation after +0xE00.

Summary before this remeasure: Clock hand as *-65536, digit fix-up over records 1..8, local reuse: 602 to 506 at delta 0. Open: register ring after +0xE00.

Summary before this remeasure: Icon x/y own locals, slide test inverted, difference after the fill: 814 to 602 at delta 0. Open: icon split-arm delay, count colour.

Summary before this remeasure: -r4300_mul + TU-local o52 data, indexed item loops, 9-record fill: 1337 to 814 at delta 0. Next: windows 0xD00-0x1500.

Summary before this remeasure: Live size 6748/0. Counted recurrence does not unroll. L160 slot/digits and L99/L100 probes inert or worse. Next: shared 24C lui and blez delay of i=0.

## 2026-10-02 h-o052b: 500 to 0, read in address order; promoted

The lap count's colour was not what rotated the tail. Giving the lap a
call-free symbol of its own puts it in v0 as shipped (a block-scoped local
does too, but its frame home pushes every uopt spill cell down 4: 530), and
the rotation after +0xE00 stays (500). The ugen freelist trace
(`DKWB_UGEN_SCHED=1`, FIFO) showed the timer arm's first draws already out
of phase, so each region was fixed in address order, keeping cells that
regress positionally when the local registers match the target:

- Second HUD pointer (`secondary = o52_bss_340[player]`) assigned just
  before its draw call; uopt still hoists it, but after the record-row base,
  so the arm entry draws match. Lap/time-difference compare written
  `racer->lap != level->laps` (level->laps is then read first, as shipped),
  and the hundredths rounding as `hundredths -= hundredths % 10;
  hundredths += o52_data_31C;` (the subtract lands in a0 as shipped). In a
  product of the three with the pointer order: 409 at +0 (the arm itself
  matched only in the 445/453 cells; 409 was a different compensation).
- Split-screen 53-item icon assigns X before Y; the full-screen arms keep X
  first (the per-arm draw orders then match): 446 at +0 (the product's 282
  cell had matching totals but wrong arms).
- Shadow alpha held unmasked and passed as `(u8)alpha` (item 16: the cast
  spends the draw that closes the ring for the rest of the function): 34.
- Item-count block in the order iconX, iconY, alpha, count store: 26.
- Lap count and time difference in iconX's symbol, alpha in its own (the
  former `difference`): 19 at +0. The slide-bar value is then a3 as shipped
  (it was s1/t0 because the time difference shared value1's symbol).

At 19 the lap count was s1 (iconX's colour) where shipped is v0. Taking it
out of iconX's symbol recoloured iconX/iconY (iconX s4, iconY s1, t2 freed
into the ring, -24 bytes). The uopt colouring records (`CDX_LOG`) said why:
iconY's web is 40/2 = 20.0, iconX alone 20.0 (wins the tie on web number),
iconX with the time difference 70/4 = 17.5 (loses), with the lap as well
150/6 = 25.0. The shipped code does keep the time difference in s1, so it
is iconX's symbol; what lifts that web is the spelling of the negative arm:

- `iconX = racer->timeDifference; iconX = -iconX;` (load, then negate the
  local) instead of `iconX = -racer->timeDifference;`: the web has one more
  def and use, and with the lap in `i` the function is 17 at +0.
- The lap then needs a symbol of its own (v0 as shipped), and so does the
  alpha (subtract v1, cast t1 as shipped); every existing symbol either
  merges a colour or adds a frame home (a fresh local pushes the uopt spill
  cells down 4: 65). The second HUD pointer is the spare: `secondary` is
  indexed at each use (`o52_bss_100[player]`, `o52_bss_340[player]`) and its
  declaration becomes the alpha's: 2 at +0. The lap store's schedule at
  +0x870 closed with it.

The last 2 words were the player*48 row offset (shared by the o52_bss_0,
A0 and 100 rows) spilled to sp+0x64 where shipped is sp+0x60. Indexing the
first HUD row at each use (`o52_bss_0[player][k]`, no `digits` for that
region; the lap and timer arms keep it) puts it in sp+0x60: 0 masked words
at +0, frame 0x118. Flat at 2 first: the toggle spelling, the lap-arm
`digits` before or after its first call. Extern declarations of the
overlay data instead of the TU-local definitions measure 1,598 words at
+16, so the data stays defined here and is dropped at POSTPROCESS.

Promotion (o054 TailA's form): the object's .data/.bss sections are removed
after its 172 section-relative text relocations are rebound to zero-valued
bases (`config/normalizations/func_overlay_052_F000063C_189ACAC.rebind.spec`),
so only overlay52Initialize's copy of overlay 52's data is linked; the 27
resident callees, the three overlay 45/56 callees and the eight resident
data names are renamed to `*_o052Reloc` placeholders in the POSTPROCESS rule.
No .rodata is emitted (every float constant is an immediate). `gmake
verify`, `check-overlay-syms` and `promotion-proof` (314/314 relocations)
pass.

## 2026-10-02 f-o052b (fourth bank): 506 to 500, and where it stops

- The screen-mode bit and the pad-buttons word share one local (both are
  s0 in the shipped code; mode dies before the player loop), the time
  difference takes the freed symbol, and the lap count and the shadow
  alpha share `i` (the shipped alpha is t1, the loop counter's colour):
  500 at +0. Aligner: 1187 byte-exact, 483 naming, 5 immediate, 12 really
  different.

Decision variable reached: the lap count's colour. Shipped it is v0; here
it is t1 (in `i`) or t0 (in any slide-bar local). Giving it a symbol of its
own (any rename that leaves its symbol otherwise unused, with the pad
buttons as an expression or in the mode local) does colour it v0, but the
first player loop then hoists `&desiredItems` and the `o52_bss_4A8[i]`
address into saved registers and the function shrinks by 24 bytes (1484 at
-24), so the symbol count is load-bearing and the lap shares a symbol in
the original. The temp-ring rotation from +0xE00 to the end, constant
(t4 t6)(t5 t7 t8), and the slide value in t0 where shipped is a3 are
downstream of the same allocation and were not separately closed.

Also measured and rejected: swapping the unkC/unk10 store order in the
first HUD arm (490 positional, but it breaks that arm's shipped schedule;
an artefact of the ring, not a fix); the item count in a named local
(eight carriers, none better than inline); the clock-hand shift spellings.

## 2026-10-02 f-o052b (third bank): 602 to 506

From 602 at +0, masked words after each edit:

- Locals reassigned (the lap count and the shadow alpha share value1, the
  time difference reuses buttons, freeing active for the slide bar alone,
  which then colours a2 as shipped): 586.
- Timer digit fix-up loop over records 1..8 (`for (i = 1; i < 9; i++)`
  with `i == 1 or 4 or 7`), so the record cursor starts at record 1 as
  shipped: 585.
- Clock hand written `racer->time * -65536 / 300`. The shipped negate into
  at followed by a shift is as1's -r4300_mul expansion of a multiply by a
  negative power of two; the four shift/negate spellings all canonicalise
  to the shift-then-negate form: 508.
- Slide bar `active = 1` after its catch-up loop: 506.

Aligner: 1181 byte-exact, 489 naming, 5 immediate, 12 really different.
The 12 structural rows are three as1 schedule points that follow from
registers: the lap count stores (lap in t0 where shipped is v0), the item
count in v1 where shipped is t0, and the slide bar's two delay slots.
From +0xF00 to the end the naming residual is one constant temp-ring
rotation (t4 t6)(t5 t7 t8); +0xE00 has value1 in t0 where shipped is a3.
Measured and flat or worse: lap/alpha/difference carrier product over 160
cells (506 best); stores and fix-up loop through the `digits` pointer
(regress by 600 or more and change size).

## 2026-10-02 f-o052b (second bank): frame homes and the icon block, 814 to 602

Masked words and size delta after each edit, in order, from 814 at +0:

- `digits = o52_bss_0[player]` after the joypad toggle instead of before
  it: 799 at +0 (the D_240 address is drawn before the player multiple).
- Slide-bar test inverted to `else if (value1 == o52_bss_4A0[player]) {
  active = 0; } else { ... }`: 800 at +0, but the bar's branch layout is
  then the shipped one (the word it costs is the upstream shift).
- Time difference assigned after the timer-sign value8 store in both arms
  (it is reloaded after the stores, as shipped): 972 at +8, the +8 being
  exactly the icon block's one-sided words.
- Icon Y given its own local declared fourth after halfHeight (its spill
  home 0x98), icon X its own local, the bar's flag and the time difference
  sharing one local, alpha reusing value2 (six locals after halfHeight, the
  shipped frame): 969 at +4. Every local has a frame home in declaration
  order; a seventh grows the frame to 0x120.
- Split-arm icon: the non-53 arm assigns X before Y: 602 at +0.

Aligner: 1085 byte-exact, 571 naming, 5 immediate, 26 really different,
displacement tax 0. Open: the split-arm `== 53` branch is a likely branch
in ours where the target fills a plain branch's delay with the else arm's
spill reload (five arm/order variants measured, none reproduce it), and the
item count loads into v1 where the target uses t0. Measured and flat:
lap count in its own local or any of six others; slide value roles and
load order (36 cells, 794 best then); icon arm order and condition sense
(16 cells).

## 2026-10-02 f-o052b: per-file flag and TU-local data, 1337 to 814

Measured with tools/fast_score.py against the configured recipe; masked
words and size delta after each edit, in order:

- `-Wab,-r4300_mul` on the object (mk/overlays.mk), source unchanged:
  1597 at +16. The hand-expanded easing recurrence is then wrong.
- Easing recurrence rewritten as a plain `for (i = 0; i < updateRate; i++)`
  over the global: 1606 at +16 (the remaining +16 was elsewhere).
- Overlay 52's .data and .bss defined in the TU as statics at their
  recorded offsets (as overlay54TailA does for overlay 54), with the 0x24C
  table split into the 2-entry arrays the function indexes: 1169 at +8.
- Item-restore loop indexed (`o52_bss_4A8[i] = desiredItems[i]`): 1112
  at +0, frame still 0x120 against 0x118.
- Item-release loop indexed (drops the `slot` cursor) and the dead `step`
  local removed: frame 0x118, 1090 at +0. Reading the stored slot back for
  the resource test and load call (`o52_bss_4A8[i]`) then gave 984 at +8.
- Timer-sign texture fill over all nine records (IDO peels one iteration
  and unrolls the other eight by four, the shipped shape), with the
  difference assigned after the loop: 814 at +0. Assigning it before the
  loop: 986 at +8.

Aligner after the last edit: 1028 byte-exact, 532 naming, 28 immediate,
104 really different; one-sided candidate words at +0x5C0, +0xCFC, +0xE14,
+0xE60, +0x1084 and target-only at +0x5B4, +0xE1C, +0xFC4, +0x15B8. All 314
relocation records now present. The residual is densest from +0xD00 to
+0x1500 (the item icon, the slide bar and the time-difference recurrence).

## 2026-09-12 exhaustive colour landscape

`residual_map.py` re-derived 672 aligned exact words and 999 aligned residual
rows (739 naming, 71 immediate, 189 structural), plus 16 candidate-only and
16 target-only words. The offsets are candidate-only
`+0x15C,+0x160,+0x170,+0x1B0,+0x1EC,+0x508,+0x5B8,+0xCEC,+0xCFC,+0xE0C,+0xE58,+0x107C,+0x1594,+0x1628,+0x162C,+0x1660` and target-only
`+0x184,+0x1C8,+0x1DC,+0x260,+0x2B0,+0x2B4,+0x2B8,+0x524,+0x5AC,+0xD08,+0xE20,+0xFC4,+0x15C8,+0x1600,+0x1610,+0x1618`. Their spans account for 306
position-only words; rank windows by the 999 aligned rows, not the 1337
positional score.

All 1,460 legal single forces were measured. The exhaustive 127 winners are:
`w105=c10/c11/c2/c7/c9`, `w122=c10/c7/c8`, `w140=c10/c11`,
`w147=c10/c11`, `w14=c11`, `w150=c10/c11`, `w162=c15`, `w165=c10/c11/c14`,
`w17=c11`, `w181=c10/c11/c14`, `w196=c10/c11/c2/c7/c8/c9`, `w204=c10/c11`,
`w26=c11`, `w284=c11`, `w295=c10/c11/c3`, `w301=c10/c11`,
`w317=c10/c11/c7/c9`, `w323=c10/c11/c18/c7/c9`, `w333=c10/c11/c18/c7/c9`,
`w351=c10/c11`, `w352=c10/c11`, `w433=c10/c11`, `w436=c10/c11`,
`w445=c10/c11`, `w453=c10/c11`, `w462=c10/c11/c6`, `w485=c11`,
`w524=c11`, `w540=c11`, `w54=c10/c11`, `w565=c11`, `w56=c10/c11`,
`w586=c10/c11/c15/c16/c17/c18/c19/c20/c21/c7/c9`, `w590=c10/c11`,
`w601=c10/c11`, `w620=c10/c11/c15/c16/c17/c18/c19/c20/c21/c7/c9`,
`w635=c11`, `w664=c10/c11/c4`, `w680=c10/c11`, `w705=c10/c11`,
`w724=c10/c11`, `w741=c11`, `w779=c11`, `w780=c11`, `w786=c10/c11`,
`w789=c10/c11/c8`, `w797=c19/c20/c21`, `w863=c19/c20/c21`, and
`w899=c10/c11`; scores range from 1195 to 1336 and are retained in ignored
footprint evidence. The report renderer's all-subset packing is infeasible at
127 winners; collapsing exact-radius rivals to 63 candidates and exact
branch-and-bound selected `w122=c7,w586=c7,w295=c3,w789=c8,w162=c15`,
predicting 1182. `force_lattice.py` measured 1184, a 153-word colour floor;
1,184 positional words (at least 846 aligned rows) remain outside it.

One bounded source experiment moved the tail-only `buttons` carrier into each
case arm to perturb the dominant reachable allocation. It preserved size but
regressed to 1,349 masked words and 1,014 aligned rows, so the retained source
was restored unchanged.

The September 8 decompiler-assisted packet retains attempt 39 as ordinary,
guarded C. This is a compiled structural plateau, not an object or ROM match.
No symbol-table, atlas, scoreboard, or matching-credit change is claimed.
The source's GLOBAL_ASM fallback remains canonical.

**Owned extent and measured result**

- Identity: overlay 52, text offsets 0x63C through 0x2098, ROM offsets
  0x189ACAC through 0x189C708, exclusive ends. The owned executable extent
  is 6748 bytes, or 1687 words; none of the deficit is padding.
- Re-derived configured baseline: 6272 bytes, 1568 words, 476 bytes short;
  1672 raw and relocation-masked positional differing words, first +0x30,
  frame 0x118, and 208 candidate relocation records against 314 runtime
  target records. The inherited 314 figure described the target surface.
- Retained result: 6736 bytes, 1684 words, 12 bytes short. It recovers
  116 words, or 464 bytes, of the baseline size deficit. The diagnostic
  comparison has 1597 raw and 1595 relocation-masked differing words,
  first +0x130. Frame 0x118 agrees; 310 candidate relocation records all
  resolve, but only 27 sites agree in offset, type, and identity with the
  314-record runtime target. Relocation exactness fails.
- All 64 direct calls, covering 31 callees, agree in identity and order.
  This is stronger than a call multiset, and weaker than matching the call
  offsets or the linked executable bytes.
- The old scalar minimum is preserved at attempt 5: 1674 words, 1451
  positional differing words, frame 0x120. It has duplicate time divisions
  whose extra words cancel missing recurrence words. Attempt 40 improves
  the retained candidate's positional count by one word but substitutes
  duplicated stores for the target addition edges. Neither displaces the
  better-understood operation graph of attempt 39.

**Recovered evidence and source**

The raw extracted assembly collapses external calls and section-zero globals.
Decoding Mickey's runtime relocation tables recovers their real overlay,
section, and symbol identities before drafting C. The stock workbench target
contains only 162 relocation records, so it cannot prove this function's full
runtime relocation surface. A section-qualified assembly adapter was checked
against the owned ROM slice: 1687 words with zero differences and all 314
relocation tuples exact. That is an importer self-check, not a C match.

The full generated context did not parse in m2c because of unrelated header
types. A narrow context using this TU's authenticated fields and call names
produced a typed draft. Raw drafts, resolved assembly, and all comparison
artifacts remain ignored. No draft or instruction listing is tracked.
The actual asm-processor output was preprocessed with the configured compiler
and accepted by the context comparator when compared with itself. This audit
was completed after the initial reconstruction, not before the baseline;
it does not retroactively establish the required preparation order.

Recovered source includes the 16-byte texture entry layout, the racer's
accessed widths and field offsets, the menu object's float fields, and the
real descriptor/time-split interfaces. The template extent includes its
ninth, terminating entry. The remaining helper return and argument widths
were checked against Mickey callee evidence. Array indexing restores shared
time quotient/remainder computation and the target texture-copy loops.
Icon coordinates are sampled before the three count draws. The clock-hand
calculation now explicitly performs its 32-bit wrap before signed division.

The opening recurrence now advances the remainder and groups of four with
an explicit exit. A counted spelling of the already expanded draft had been
unrolled a second time; the explicit exit avoids that expansion. Its opening
call interval grows from 76 to 106 words against the target's 108, recovering
30 words in the intended region. Each positive update count still performs
exactly that many f32 subtraction/multiplication/addition steps and global
stores; nonpositive counts perform none. There is no extra final floating
operation, invented guard, or padding. This reconstruction does not prove
that the original source manually grouped the updates.

**Remaining deficit and stopping evidence**

Call-delimited extent differences are now confined to the opening recurrence
interval (two words short), item replacement (one short), item restoration
(two short), the main icon draw (one extra), and the final count draw
(one extra). Equal extent elsewhere is not byte identity.

The recurrence still has two ordinary backedges where the target uses likely
backedges with duplicated addition edges. Its FP carrier graph differs.
The initial paired halfword stores also form separate high-address loads,
where the target shares them. Other relocation-count differences concern
address formation for fields of the same authenticated entry arrays; there
are no unresolved callee or global identities. Remaining icon and item
intervals include different address/value lifetimes and spill placement.
No wrong arithmetic literal was found in the workbench constant bucket;
its aligned constant findings were stack-displacement differences.

Attempt 39 was the last accepted structural gain. After the single-word
positional alternative at attempt 40, five consecutive refinements left the
retained residual and the remaining causal explanation unchanged:

- 41: separate old-value and delta locals add copies, six words, and a
  larger frame; additional relocation sites are not the target sites.
- 42: one accumulator for delta and result retains the wrong latch and
  does not improve the residual.
- 43: a forward completion exit changes scheduling but leaves the same
  opening extent and relocation deficit.
- 44: a typed float cursor removes target loads/stores and grows the frame.
  Its lower positional count is a regression in the structural comparison.
- 45: counter initialization on the nonpositive path is byte-flat.

These refinements recover no new call/global identity and do not resolve a
new mismatch mechanism. Stop on that stall, not on the total attempt count.
Earlier named-step, simple loop-condition, countdown, and pointer-hoist forms
are preserved as exhausted inputs. The remaining recurrence issue is not a
missing update operation. Do not restart those spellings without changed
evidence. No flag lattice, permuter, instrumented compiler, invented volatile
access, global regrouping, or post-compile instruction editing was used.

The concrete next lever is a faithful comparison of the producing compiler
phases for the retained recurrence and a Mickey-proved sibling with the same
branch-latch shape, plus an address-context check for the paired fields.
First establish whether source def/use structure or compiler context owns
those decisions. This requires a separately authorized mechanism; it is not
permission to change shared flags, symbols, alignment, or data placement.
The workbench verdict remains structure-mismatch, routed through
structure-buckets: context parity and the spill-slot census, after the
completed constant and call/global identity audits.

**Artifacts, commits, and validation**

The packet base is cbd755d9b2e828b5f4c05a4efe81d70353d205cb.
Progress commits are 0f5059c5a702933b4a3c3faf6fdc1fdfa8b61d17,
c45403aa, and 0f914be9; the commit containing this handoff banks the grouped
recurrence and final interface corrections. Only the owned source and this
shard change in that final source commit.

Ignored evidence is under build/whale4/: baseline source/object and reports,
attempt snapshots through a045, the retained a039 source/object/diagnostic
ELF, per-attempt rationale and measurements, runtime identities, the target
adapter self-check, compiler-input self-comparison, and recurrence review.
The standalone link is a stored-addend diagnostic, not a synthetic-address
promotion or a canonical linked-ROM proof. Every meaningful alternative is
preserved, including the scalar minima.

Commands used include generate_ctx.sh, mips_to_c.sh with the typed local
context, tools/wb_compare.sh with the configured full-TU candidate,
decomp-workbench diagnose/compare/guide structure-buckets,
tools/candidate_context.py, and the ignored measure.py/call_buckets.py
adapters. tools/finalize_plateau.py generated the guarded plateau metadata
and this shard's metric header; the richer evidence is recorded here.

Source gates and all 243 NON_MATCHING TU builds passed at each banked
checkpoint. gmake verify repeatedly fails in unchanged overlay94UpdateController
at the assigned base: undefined func_800254FC, func_8002565C, func_8002A878,
and func_8002A910, plus resident R_MIPS_26 overflows for func_8005ABA8,
func_8005AF14, func_80019AB8, and func_8002B040. There is no canonical linked
ELF available for an owned-range extraction. This remains outside this
lane's ownership. Closing gate receipts are retained alongside the packet;
no full-ROM success or newly matched bytes are claimed.


**Residual partition (2026-09-10 measurement pass)**

The retained candidate was re-measured against the extracted target under the
configured translation-unit flags. All 1687 words partition exactly, and the
partition contradicts the assumption that a function this far out is
structurally wrong:

- 349 words byte-identical in place.
- 379 words byte-identical to their aligned partner but in the wrong position.
- 699 aligned pairs differing in register fields only.
- 119 aligned pairs differing in register fields and immediate.
- 81 aligned pairs differing in immediate only.
- 60 target words with no aligned partner.

The first bucket plus the other five is 1687; the last five is the 1338 that
differ positionally. Alignment here means the opcode-and-shape correspondence,
computed by matching the two instruction streams on opcode, function code,
shift amount and format field with every register, immediate and branch
displacement removed.

*The operation mix is already right.* Counting every instruction class across
both sides, only seven classes disagree at all and the total instruction-count
disagreement is twelve out of 1687. High-half materialisations, calls,
comparisons, stores and floating multiplies agree exactly in count. This is not
a different program; it is the same program with different register names and a
small number of placement defects.

*All displacement comes from three call intervals out of sixty-four.* Splitting
both sides at their direct calls, sixty-one of the sixty-four intervals have
identical word counts. Interval 8, the opening float recurrence, is two words
short; intervals 36 and 39 are one word long each; the three cancel, which is
why the extent is exact. The two words in interval 8 are two floating additions
that accompany the target's use of the likely branch form on both recurrence
back edges, where the candidate uses the ordinary form and fills the delay from
the instruction before the branch. This is an assembler scheduling difference,
not a missing operation: both loop bodies hold the same six operations in the
same cyclic order and differ only in which one is rotated into the delay slot.
The extra word in interval 39 is a second reload of a spilled local that the
target reloads once, because the target's reload lands in a callee-saved
register that survives the intervening calls and the candidate's lands in a
caller-saved one. Repairing those three intervals is worth about 390 words:
under a piecewise constant shift the count drops from 1338 to 988, and under
free alignment to 948.

*The rest is register naming, and part of it is a uniform ring rotation.* Under
the opcode-and-shape alignment, 818 of the 899 differing pairs disagree in a
register field. Rotating the candidate's temporary ring by a fixed number of
steps recovers a large share of them. A three-segment model, no rotation to
about +0x540, one step from there to about +0x9C0, no rotation to about +0xF60,
two steps from there to the end, raises the identical count from 728 to 924.
The second segment alone is 91 words and the fourth is 68. Fitting one
arbitrary register substitution for the whole function instead raises it to
1037, and allowing the substitution to change every 128 words raises it to
1316, which bounds how much of the naming difference is relabelling rather than
different code.

The same slip shows in the frame. The frame size agrees and the number of homes
agrees, but the candidate's home block sits one four-byte cell below the
target's over a long span while one later home sits one cell above, so 54 of
the 67 disagreeing stack displacements are exactly four bytes. Home order is
web-creation order, so this is one temporary created one position out of turn,
and it is very likely the same decision as the one-step ring rotation.

*Relocation representation is not a factor.* The project ranking already
reports 1338 raw against 1337 relocation-masked. Resolving every overlay-local
high and low half on both sides to its numeric address independently changes
the aligned identical count from 728 to 735. Mickey's overlay text stores
addends and the extraction re-symbolises them, so the extracted target and the
unlinked candidate object are directly comparable. Twenty-four candidate sites
do carry a nonzero stored addend where the target stores the same address as a
symbol reference with none; ten of those are byte differences the source
spelling can remove by naming the object at that address instead of an offset
from an earlier one.

Falsified during this pass, each by a paired build under the configured flags:
equality-comparison operand order, canonicalised to byte-identical output;
commutative operand order on the recurrence multiply, byte-identical; an
explicit non-compound assignment for the accumulator, byte-identical; giving
the remainder or the counter its own local, which changes the frame and worsens
the residual, so the current local census is the right one; and the top-tested
counted loop forms, which unroll and add twenty-eight words.

One diagnostic result is worth recording even though its spelling is not
adoptable. Replacing both recurrence exit tests with an exclusive-or against
zero leaves every instruction in the recurrence unchanged and still moves 764
words of the candidate downstream of +0x300, dropping the positional count from
1338 to 1220 and removing the two-step ring rotation in the tail. That proves
the tail's rotation is set by a web count upstream rather than by anything in
the tail, and that one more web consumed in the recurrence is enough to correct
it. It is not a proposed source form.

The next lever is therefore the web-order slip, approached from the frame-home
census rather than from the recurrence's operation graph, which the earlier
packet exhausted. `gmake verify` passes in this worktree at this commit, so the
link failure recorded in the previous section no longer reproduces.


#### Phase census, the two-region partition, and the allocator's real share (2026-09-11)

**Census first: this translation unit compiles to a single uopt procedure, and
its 237 globalcolor decisions are all phase one -- 228 integer, 9 floating,
zero phase two.** 104 are coloured and 133 split. So the phase-two
web-numbering axis is not live here and no amount of moving a defining
statement is a colour lever for this function. The rule is general: over the
61 procedures of this unit, `overlay_001_tail.c` and `overlay_008.c`, every
procedure that issues a call emitted phase-one records only and every leaf
emitted phase-two records only, 59 classified with no counterexample
(`docs/ido-learnings.md`).

Aligned against the target on a register-erased shape
(`tools/align_symbol.py`): 1687 words against 1687, positional 1337,
displacement tax 306, and the residual splits 672 byte-exact / 739
register-naming / 292 really-different. The extent agrees but not locally --
16 candidate words in 14 spans and 16 target words in 14 spans, which is why
the extent cancels.

**The residual is two independent problems with different owners, and the
earlier packet's segment model is one of them.**

*Region 8, the opening float recurrence, +0x130 to +0x2E0.* 113 words, 19
byte-exact, 54 naming, 40 really-different. Every one of the 54 naming rows
is a floating-register difference; there is essentially no integer naming
before +0x400 (nine field substitutions in the whole prefix). This region is
the only place in the function where float allocation is in question at all --
the aligner finds float differences in just 4 of the 65 call-delimited
regions.

*The integer body from +0x400 on.* 622 of the 739 naming rows name a register
`globalcolor` never assigns in this procedure. The trace is explicit about
which registers it does assign: `v0`, `v1`, `a0`-`a3`, `t0`, `t1`, `t2` and
`s0`-`s8`, 104 colourings in all of which four are floating, and **never
`t3` through `t9`**. Those seven
are ugen expression temporaries, and the residual is a piecewise-constant
permutation of them:

  - +0x0 to +0x400: nine integer field substitutions in the whole prefix;
    clean.
  - +0x400 to +0x800: the seven-cycle `t3`->`t4`->`t5`->`t6`->`t7`->`t8`->
    `t9`->`t3`, a one-step rotation, carrying 93 of the window's 107
    substitutions across 78 naming rows.
  - +0x800 to +0xC00: a transition; both permutations are present and neither
    dominates.
  - +0x1000 to the end: a five-cycle `t3`->`t5`->`t7`->`t9`->`t6`->`t3` with
    `t4` and `t8` transposed, carrying 497 of the window's substitutions
    across 376 naming rows.

  Over the whole function that second permutation alone accounts for 586 field
  substitutions, which is why an arbitrary single relabelling recovered so much
  of the residual for the earlier packet and a rotation of a single ring did
  not: it is a five-cycle and a transposition, not a rotation.

Only 48 of the 739 naming rows are purely among the registers globalcolor
assigned, and 19 of the 69 rows with a float difference have both sides inside
the class-2 pool (c24=`f0`, c25=`f2`, c26=`f12`, c27=`f14`, c28=`f16`,
c29=`f18`; decode in `docs/ido-learnings.md`). So only about 67 of the 739
naming rows are a colour globalcolor actually picked -- though that is a
statement about what *kind* of decision each row is, not a bound on what the
allocator can reach, because the ring's phase is itself downstream of how many
pool colours are consumed (the force ceiling below closes 156). The earlier
packet's ring-rotation reading is confirmed and its owner is now named:
it is ugen's temp ring, not a colour, and the ring's phase is set by how many
temps are consumed upstream -- which is why that packet's exclusive-or
diagnostic in the recurrence moved 764 words in the tail.

**Where the naming actually is.** Four consecutive call intervals hold more
than half of it: region 40 at +0xE9C (162 words, 47 exact, 101 naming), region
41 at +0x1120 (143 words, 45 exact, 98 naming, and *zero* really-different),
region 42 at +0x135C (200 words, 61 exact, 116 naming) and region 43 at
+0x166C (112 words, 39 exact, 68 naming). 617 words, 383 naming rows, 42
really-different. Region 41 is the cleanest target in the function: 98 words
wrong, all of them a register name, nothing structural at all.

**The really-different bucket is mostly not different code.** 71 of the 292
rows are the same instruction at a different `sp` displacement, 49 at a
different non-`sp` displacement, 57 the same mnemonic with another immediate,
83 a genuinely different opcode, and 32 present on one side alone.

**The frame is one home swapped in creation order.** Both frames are 280
bytes. The candidate's home block at +0x5C through +0x70 should be +0x60
through +0x74 and its home at +0x9C should be at +0x98: the shipped frame
leaves +0x5C empty and uses +0x74, the candidate uses +0x5C and leaves +0x74
empty, and the two swap back at the later home. 36 of the 71 `sp` rows are
exactly +4 as a consequence. That is the earlier packet's "one temporary
created one position out of turn", now read off the home set directly rather
than inferred from a displacement histogram.

**How much of this is the allocator's to give.** Greedy force ceiling,
sweeping all 213 webs against every colour and the split path each round and
keeping the best (4,764 compiles per round):

  - baseline 672 byte-exact / 739 naming / 292 different, positional 1337;
  - `p1:w877=s` -> 806 / 605 / 292, positional 1210. This is a single force
    worth 134 byte-exact words. Web 877 is one of eight webs uopt derives from
    its symbol 102, a local with 54 references; its first decision already
    splits, and it is the *second*, `save=30.0`/`nocs=1` piece that the
    candidate colours and the target does not;
  - `+p1:w882=c7` -> 821 / 590 / 292, positional 1195;
  - `+p1:w462=c6` -> 828 / 583 / 292, positional 1189.

Three rounds recover 156 byte-exact words, 15% of the 1031 wrong ones, and the
increments are 134, 15 and 7, so the series has effectively converged. The
same sweep on the other two whales converged at 16% and 17%, so a sixth is the
number to budget. **The really-different bucket does not move by a single word
under any force**, and the integer ring permutation survives all three rounds:
what the forces buy is bought inside the naming bucket, and not enough of it to
change the verdict.

**Recommended order for the next attempt.** Region 8 first -- it is upstream of
the ring phase, it is the only float region, and the earlier packet already
showed that consuming one more web there corrects the tail rotation. Then the
one swapped home. Region 41 is the cheapest confirmation that the ring moved,
because it has no structural content to confound the reading. Do not open a
declaration-position or statement-position lattice: the census says that axis
is dead for this function.

Nothing above is a source change, a match claim or a credit claim. The
`GLOBAL_ASM` fallback remains canonical and `gmake verify` passes at this
commit.

#### 2026-09-19 lane w29-o052: size already closed; listed size levers are negatives

Live re-measure on `386b918d`: 6748 bytes, size delta 0, 1687 vs 1687 words,
masked 1337, frame 0x118 both sides, 42 slots both sides, first mismatch
+0x130 (immediate), first structural +0x13C. Aligned 672 exact / 739 naming /
71 immediate / 189 structural plus 16 candidate-only and 16 target-only
words. Displacement tax 306. Identity-gated instrumented IDO `.text` matches
stock; `CDX_PROC=0`, 237 p1 decisions. Candidate object has 310 relocation
records.

The inherited 12-byte-short figure is stale. `web_footprint.py --every-colour`
is therefore legal on size, but the colour packing floor of 1184 was not
re-run (source unchanged).

Falsified, each by a configured full-TU compile against the same comparator:

- A plain `for (i = 0; i < updateRate; i++)` float recurrence, with or
  without the `updateRate > 0` guard, does not unroll. Size -112 / -120
  (28-30 words short). The unroller that produced `func_80005548`'s
  remainder-plus-4x copy loops does not fire on this carried global float
  update. Do not retry a counted spelling of the recurrence.
- Cleanup bound spelled as `(s32 *)&o52_bss_4B0` instead of
  `o52_bss_4A8 + 2`: size -4, exact 569.
- Restoration loop as `o52_bss_4A8[i]` without the walking `slot` cursor:
  size -8, exact 636. Combined with the bound change: size -12, exact 574.
- Nested `if (D_800C947C == 0) { i = 0; if (updateRate > 0) ... }` to put
  `i = 0` in the `blez` delay: size 0, masked 1337, exact 671 (one worse).
  It drops the +0x170 / +0x184 insertion pair and does not pay.
- Comma-assign of the paired `o52_data_24C` stores: byte-identical.
- Leftover `updateRate or-equals 0` at entry: byte-identical.
- Empty `if (i) {}` after the recurrence: byte-identical.
- Deleting the `digits` cursor for generated `o52_bss_0[player]` /
  `o52_bss_A0[player]` / `o52_bss_200[player]` subscripts: size 0, masked
  1341, exact 675, but two new insertion pairs, first immediate +0x30, and
  a worse home set.
- Unused `f32` after `step`: frame 0x120, masked 1349.

The first two candidate-only words remain +0x15C and +0x160: each arm of
the `o52_data_24C` stores rematerialises `%hi` instead of sharing one
`lui` while `-120` is live. Target fills the `updateRate > 0` `blez` delay
with `i = 0` and copies the remainder (`andi` then `or`); the candidate
fills that delay with the `andi` and never copies. Those two as1 identities
are still the next lever. Colour packing is not.

No source change is retained. `GLOBAL_ASM` remains canonical.
<!-- plateau-handoff:func_overlay_052_F000063C_189ACAC:end -->
