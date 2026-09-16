<!-- plateau-handoff:func_80046BCC:start -->
### `func_80046BCC` plateau handoff

- source: `src/main/diCpu.c`
- score: 16/106 words
- frame: 0x40
- relocations: 3
- first mismatch: +0x2C
- summary: Seven-draw census confirms four-window live-range split; exhausted source and colour routes remain at 16.

#### 2026-09-09: the ninth callee-saved web is the working copy

41 differing words to 31 at 106/106 words, frame `0x40`, all three relocation
sites exact.

The previous pass folded the m2c draft's `var_s0` into `var_s2` to close a +4
size mismatch. That was right about the size and wrong about the register
file: the fold frees a callee-saved register, so the candidate hoists a third
loop-invariant constant (`0x78`) into `s8`, where the target materialises it
inline with `li at` and hoists only `0xA` and `0x30`. The copy is a real
source variable, not the allocator's artefact: `var_s0` carries the character
through the range tests while `var_s2` keeps the value the next iteration's
`temp_s6` reads. Reinstating it puts `s7 = 0xA` and `s8 = 0x30` back in the
target's registers and removes ten differing words.

Remaining: one live-range split. The target computes the masked character
straight into its callee-saved carrier and splits a copy into `s0` for the
range tests; the candidate computes it into a caller-saved temporary, runs
every range test out of that temporary, and copies into the saved carrier.
That one decision also exchanges `var_s2` with `var_s3` and `var_v0` with its
own temporary, which is most of the 31. The instruction count, the frame, the
branch structure and the relocation sites all agree.

Measured flat or worse against it in this pass: both orders of the
`var_s2 = 0` / `var_s3 = 0` initialisers (33); all legal orders of the three
loop-head statements (31, 32, 32); `u8 var_v0`; the loop test written as
`while ((var_v0 = *var_s4) != 0)`; the range tests spelled entirely on either
variable; reversed equality operands at two sites; the copy written after the
case block (41), inside each arm (41), and as a plain working copy with one
write-back (40); and three declaration orders. Resume on why the mask lands in
a caller-saved temporary here and directly in the saved carrier there.


#### 2026-09-09 (second pass): the live-range split is a compiler temporary

The previous pass asked "why does the mask land in a caller-saved temporary
here and directly in the saved carrier there". The instrumented uopt answers
it. The masked character is a web of its own -- phase-one web 32 -- and it is
coloured *first*, ahead of every declared local, so it takes v0; `var_v0` is
web 0 and gets v1 behind it. The target has no such web: its mask writes the
callee-saved carrier directly.

Two force probes bound the space:

- `CDX_FORCE=p1:w0=c1`, putting `var_v0` back in v0, is declined twice --
  web 32 already holds v0 and the two interfere. The register file cannot be
  recovered by moving `var_v0`; web 32 has to stop existing.
- Forcing web 32 into a callee-saved colour reaches the target's registers and
  wrecks the schedule instead (73 and 74 differing words for s2 and s3), and
  forcing its split path costs two instructions (108 words against 106).

Web 32 exists because `var_s2 = var_v0 & 0xFF;` is immediately followed by
`var_s0 = var_s2;` in the same basic block. The value has two destinations, so
uopt commons it into a temporary and copy-propagates both names onto it -- which
is also why every range test in the candidate reads the temporary rather than a
carrier, while the target tests the first bound on its saved carrier and the
second on the copy.

Newly eliminated this pass: nested `if`s in place of the two `&&` pairs (31,
byte-flat); `var_s0` spelled as a second `var_v0 & 0xFF` (31, flat); the copy
moved ahead of the pointer increment (32); the copy pushed inside each arm with
an `else` copy on the short path (103, and the frame moves); `var_s0 = var_s2 &
0xFF` (89); both range tests on `var_s2` with the copy after the case block
(41); `var_v0` masked at the load (86); and `temp_s6` taken from `var_s0` (56).

Resume by removing the mask's second destination, not by reordering it: the
target's source cannot be creating a second name for that value in the same
block. Everything else -- instruction count, frame, branch structure, the three
relocation sites and the two hoisted constants -- already agrees.

#### 2026-09-12 (lane `lane/p7-res2`): the three-variable family is closed at 192 forms

Still 16, register-only at 106/106 words with the frame, the branch structure
and all three relocation sites exact. The 2026-09-11 note closes on "separating
them again brings web 32 straight back: all four three-variable forms measure
31, 33, 89 and 91". Those four were re-tested as a lattice rather than four
points, and the closure holds with a great deal more force than it was written
with.

**192 three-variable forms measured, every one between 83 and 91.** The lattice
crosses eight declaration positions for the reinstated working copy, three
spellings of the copy itself (a plain copy of the masked character, a recomputed
mask of the loaded character, and an or-with-zero barrier), both operands for the
first range bound, both assignment orders inside the conversion body, and an L97
`if (1)` region around each arm's body present and absent. The distribution is
eight cells at 83, eight at 84, forty at 86, eight at 87, forty-eight at 88,
forty at 90 and forty at 91 -- no cell anywhere near the two-variable 16, and
none below 83.

The cause is visible in every one of them and is the same cause the earlier note
named: giving the working copy its own name gives the character mask a second
destination, uopt commons it into a temporary, the temporary takes the first
caller-saved colour and the loaded character is pushed one place down the pool.
The or-with-zero barrier does not prevent it, which is L135 doing exactly what
it says -- uopt folds the identity operation before the web builder, so the
probe is not a probe.

**So the decision variable is unchanged and now has a much wider negative under
it**: the loaded character's live range has to stop before the working copy's
begins without a second IR name existing, and no declaration, copy spelling,
bound operand, assignment order or region opener in this family does that.
Resume on something that splits one symbol's range rather than on adding a
symbol -- L131 is explicit that differing *spellings* at the def sites, not
differing locals, are what split a range.

Two smaller facts for whoever resumes. The remaining 16 words are not one
window: `register_census` reads 68% global coherence over five source registers
with four windows opening at +0x78, +0xAC and +0x164, so this is per-iteration
consumption and each window is its own question. And two of the sixteen are pure
comparison operand order against the two loop-hoisted constants -- the candidate
puts the character first and the target the constant first at both sites -- which
is a smaller and separable question from the twelve-word pool rotation.

#### 2026-09-12, lane `p12-tight`: the whole callee-saved bank is swept by force, and nothing reaches 16

Baseline reproduces: 424 bytes, 106 of 106 instructions, delta 0, frame 0x40,
16 relocation-masked words, first mismatch +0x2C. Aligner: 90 byte-exact, 16
register naming, 0 immediate only, 0 really different -- a pure allocation
residual with no structural component at all. No edit adopted.

**The instrumented toolchain's identity gate was taken on this TU first**: its
text section is byte-identical to the tree's object for `src/main/diCpu.c` at
the configured flags.

This procedure is globalcolor ordinal 11 and records 18 decisions, all phase
one, which is the call test reproduced. Its callee-saved ladder is nine coloured
webs in decreasing save -- 25.78, 11.10, 10.10, 4.56, 3.10, 3.10, 2.50, 1.11,
1.11 -- followed by six more webs at 1.11 that all split for want of a colour.
Two of those saves tie, and the tie goes to the lower web number, as recorded.

**135 single forces, none below 16.** Every one of the fifteen webs in that
ladder was forced against every one of the nine callee-saved colours, with
`CDX_PROC` set and acceptance read off the record's `forced` field rather than
off whether the object changed. The diagonal reproduces the incumbent; every
cell above it is 18 to 56; every cell below it is declined because an
earlier-decided web already holds the colour. Eight further paired forces,
including the exact two-cycle `register_census` reports over the first and
third callee-saved colours, are 18 to 46.

That is a stronger statement than the shard has carried: **the sixteen words are
not a globalcolor decision at all.** No assignment of this procedure's webs to
this procedure's colours is closer than the incumbent, so the target does not
have these webs. It has a different web SET, which is the same conclusion the
192-form three-variable lattice reached from the source side, now confirmed from
the allocator side.

**L151 does reach this function, and it does not help.** The 2026-09-12 law that
a literal's type is part of its IR identity was tested against the named blocker
-- the working copy being commoned with the character mask. A copy spelled with
an `unsigned`-typed mask, a cast to `u8`, a 255 written unsigned, and a mask
cast through `u32` each DO split the web: the size penalty falls from +8 bytes
to +4 and the structural bucket from 7 to 5. But every one of the eight
spellings measured stays at 91 or 92 words, against the two-variable 16. So the
three-variable family is closed under the constant-type axis too, and the axis
is confirmed live in this compiler rather than merely untried.

`register_census`, re-read: 68 per cent coherence, five source registers, four
windows opening at +0x78, +0xAC and +0x164, with the first callee-saved colour
mapping to the first caller-saved one at six sites in one window and to the
third callee-saved one at six sites in another. A register that corresponds to
two different target registers in two windows is a live-range SPLIT in the
target that the candidate does not have, not a colour the candidate got wrong.

**Next instrument, named.** The freelist trace, `DKWB_UGEN_SCHED=1
DKWB_UGEN_TRACE=1`, stamped per source line (L149). It is the only instrument
that sees a per-iteration consumption difference, it has never been run on this
function, and the force sweep above has now eliminated the pass above it.

#### 2026-09-12, lane p15-res: the ugen trace reduces the search to four sites

The configured instrumented compile is text-identical to stock and identifies
this procedure as ordinal 11. Its GP freelist has exactly four source-line draw
sites: the upper-case fold, lower-case fold, tab alignment, and glyph-call
address expression. The tab alignment alone consumes the draw at its source
line; rewriting it as a compound subtraction assignment is byte-identical, so
that equivalent spelling neither changes the draw nor any residual window.

The 16-word residual remains four incoherent windows, so this trace does not
identify one global phase correction. The remaining source search should test
only semantically faithful forms at the other three stamped sites, rather than
another allocator or declaration-order lattice.
#### 2026-09-12, lane p23-lastmile5: complete single-colour landscape

Fresh configured baseline: 106 words, delta zero, 16 masked and 16 raw
mismatches, first +0x2C. Aligned buckets remain 90 exact, 16 naming, zero
immediate and zero structural; there are no surplus or missing words. The
0x40 frame and three relocation sites are retained. Stock versus instrumented
full-TU fidelity passes text, data, rodata, symbols and relocations.

A retained Ucode/name receipt authenticates procedure 11 of 14.
Procedure 11 has 12 coloured webs. The exhaustive same-save-kind landscape
contains 66 compiled legal alternative forces and one singleton web with no
second colour; every compiled force was accepted. The complete winners list
is empty. Nine forces were byte-flat in masked score and the others regressed.
There is no improving packing to pass to the combination lattice: the measured
single-force floor, including the unforced member, is 16. This does not prove
an unrestricted multi-force or source-shape lower bound.

Nine stock source probes also retain 16 at delta zero. Three split the
conversion addition/subtraction through the existing saved-character carrier
before masking (upper arm, lower arm, both). Three respell the glyph address
as subtraction-before-scaling, pointer arithmetic, and unsigned literals.
Three use a byte cast or signed/unsigned masked conversion spellings in both
fold arms. Each source and object is retained privately under
`build/p23/func_80046BCC/`; no candidate was adopted. The final three conversion
type attempts give no improved residual or new identity, satisfying the stall
rule. The source question remains how to separate the initial character load
from the later working-character live range without introducing the already
refuted extra mask destination. Neither this complete colour axis nor the
three stamped expression sites supplies that split.

#### 2026-09-12, lane p24-recipe: removing load and cursor carriers

The stock baseline reproduces 16 raw/masked differences in 106 words, delta
zero: 90 exact, 16 naming, zero immediate and zero structural rows. The frame
is 0x40, with three relocations and first mismatch +0x2C. The existing fully
authenticated 12-web, 66-alternative colour landscape was reused unchanged.

Three source forms test the deletion recipe without another three-variable
copy lattice. Reading the byte directly at the loop head and loop condition
removes the declared load carrier but adds a load at +0x48: 91 masked, delta
plus four. It removes three late naming rows but adds nine aligned residual
rows overall. Indexing text instead of carrying a walking pointer is 20 at
delta zero, with the original conversion windows unchanged. Advancing the
text parameter directly after deleting its pointer alias is 18 at delta zero,
adding two entry naming rows and leaving every other window unchanged.

All three fail to improve the retained sixteen-word residual or establish the
wanted target split. Stop under ADR 0018; retain the original guarded source.
The recipe's limitation here is explicit: deleting the load name duplicates
the memory read, while deleting the pointer name changes only entry allocation.
Neither separates the loaded byte from the later working character at zero
instruction cost. Source, object, score and aligned comparisons remain private
under build/p24/func_80046BCC. A scratch-path canary reproduced the configured
baseline before those retained-object comparisons were used.

#### 2026-09-16, lane lm-c: every declared working copy hoists and commons

Baseline reproduced at 16, delta zero, first +0x2C, register-only. Read off
both objects: the target keeps the loaded byte caller-saved and never
across the call, the working char in the first saved register, and the
arm reads add into the second saved register, mask into the first, then
copy back; the copy ahead of the range test is not propagated into it.
The procedure-11 census confirms ours has one symbol web for the loaded
and working char (web 0, 17 interferences) spanning the loop. Four
cycles, 33 cells:
- the three-statement arm spelling is byte-identical at 16; on `var_s2`
  alone 38.
- a declared working copy in either width, copied inside the arms from
  `var_s2` or from the masked load, tests mixed or on the copy: 16 cells,
  all +4 to +24 bytes. The surplus word is a copy at +0x58: both arms'
  copies hoist into the predecessor and common with the mask.
- anti-commoning spellings of the two copies: +4 to +12.
- no working copy at all, arms and tests on `var_s2`: 42 at delta zero.
Next: the target's saved working register is most likely uopt's own split
of `var_s2`, which is precisely a copy that is not propagated; the lever is
that web's split gate, not a source variable. Read its record.

#### 2026-09-16, lane w1-a: one symbol web, and the copy's value number

Baseline reproduced at 16 masked, delta 0, first +0x2C, register-only.
Three batches, 17 cells, none below 16; the source is unchanged.
Report: [lastmile-web-membership.md](../lastmile-web-membership.md).

The procedure-11 records: `var_v0` is one symbol web (web 0, blocks 0 to
33, nocs 9, totalsave 232, coloured s0) over both the loaded chain and
the working chain; uopt does not rename them apart. The ROM keeps the
loaded byte caller-saved and dead before the call, so its source has two
symbols. Read off the objects, the sixteen rows are the loaded byte's
register (six rows), the second range test reading the working char
where ours reads `var_s2`, and the fold's mask and copy-back exchanging
registers (the ROM masks into the working char and copies to `var_s2`;
ours the reverse).

- The loop-bottom load in the loop condition, behind a region opener, or
  inside its own region: 16, inert. Block placement of the load is not
  what keeps the chains one symbol.
- A separate loaded-byte symbol (`s32` or `u8`) with the working char
  kept as `var_v0`: 91, +4. The object says why, and it corrects the
  hoisting reading: the arm copies are not hoisted, they stay in the
  arms; uopt value-numbers `w = var_s2` with `var_s2 = c & 0xFF` and the
  masked value gets three destinations, a temp plus a copy into `var_s2`.
  The base avoids it only because the mask's operand is the copy's own
  symbol.
- `u8` working char with the three-statement fold and no explicit masks
  (two-variable and three-variable, `s32` or `u8` loaded byte, the masked
  fold too): 79 to 94, +4 to +16. The truncations are emitted.
- `u8 var_s2`, the lazy-normalisation reading of the ROM's fold: 88 to
  94, +4 to +20. The masks are emitted eagerly on the store, on the read
  into the working char and on the copy back.

Next: the decision variable is the value number of the arm-head copy. In
the ROM the copy reads `var_s2`'s register, so its value is not the
mask's: either `var_s2` has a second reaching definition at the arm
heads or the loaded byte is unavailable there. The untested cell is a
zero-width redefinition of the loaded symbol between the mask and the
arms; a plain two-symbol form on this shape is closed by the 192-form
lattice, lm-c's 16 cells and the five here.
<!-- plateau-handoff:func_80046BCC:end -->
