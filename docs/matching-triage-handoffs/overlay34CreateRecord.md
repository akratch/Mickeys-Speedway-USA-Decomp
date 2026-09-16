<!-- plateau-handoff:overlay34CreateRecord:start -->
### `overlay34CreateRecord` plateau handoff

- source: `src/overlays/o034/overlay34CreateRecord.c`
- score: 2/125 words
- frame: 0x30
- relocations: 12
- first mismatch: +0xE8
- summary: uopt closes the store block after twenty local loads, one per store and one per stored variable, which leaves height's last store in a second block; two self-defining reads of height there outrank the resource copy (6 to 2), and the byte12/short16 pair is that same boundary read the other way.

#### 2026-09-16, lane lm-a: two cycles, no source reaches the ranking

Baseline reproduced: 6 masked at delta zero, aligned 119/4/0/2, one
`a0`/`v1` cycle at `+0x88..+0xA4` and the byte12/short16 store pair. The
records say the four naming rows are the ranking already named -- the
resource copy (web 44: save 3, `nocs` 1, block 10) is decided before
height (web 50: save 2, `nocs` 2, blocks 10 and 11) -- and height's second
block is the open question: width (web 42) has all its references in block
10, and the constant `1` web reaches block 13 through `active = 1`, so the
store sequence is not one uopt block, but nothing in the source names the
boundary. Seventeen cells, all at or below the retained form: routing the
null test, the width load or the height load through the loop's `record`
or `current` aliases (119-124 words, +4 to +24 bytes: the aliases are
separate webs and referencing them after the loop extends their ranges);
splitting or multiplying the height intermediate (16, 12); `s16`/`u16`
dimensions (50, 102); the mode multiply moved to the end (39); height
declared or computed before width (6 and 92). Next: find the block
boundary inside the store sequence from the instrumented uopt's block
table before trying any further order, because every order cell so far
has been measured blind to it.

#### 2026-09-12, lane p9-mid: 24 -> 8 on the globalcolor records

The closure above asked for "direct new evidence for the missing producer/home
identity". That framing was wrong: the frame and every home have been exact for
some time, and the instrumented globalcolor records show the whole residual was
**rank order among webs that all get a colour**, not a missing home. Nothing here
needed a home, a donor, or a flag.

Aligner buckets before: 101 byte-exact, 22 register naming, 0 immediate-only, 2
really different, at 24 masked and size delta 0. After: 117 byte-exact, 6
register naming, 0 immediate-only, 2 really different, at 8 masked and delta 0.

The instrumented toolchain's `.text` for this TU is byte-identical to the tree's
object, and this TU holds one function, so the procedure ordinal is 0. Every
force below was verified accepted by reading the `forced` field, not by the
object changing.

Three edits, each priced by a force first and then reached from source:

- **The structural pair was L111, not a home.** Writing the cursor advance as
  `record = current + 1` instead of `record++` keeps the `current = record` copy
  as a real node. That gives as1 a third node in the loop's test block, so it
  fills the branch delay slot with the index compare where the candidate had a
  `nop`. Structural bucket 2 -> 0 at an unchanged 24.
- **An L109 or-with-zero probe on the index is worth 6 words.** `CDX_FORCE=p1:w13=c2,p1:w10=c1`
  scored 17 against 24 at delta 0. The two webs are the record cursor
  (totalsave 32, nocs 2, save 16.0) and the loop index (totalsave 31, nocs 2,
  save 15.5), and p1 ranks by `save`, so one unit of totalsave decides which
  takes `v0`. The probe raises the index web to totalsave 51 at unchanged nocs,
  save 25.5, and the probed object is **byte-identical to the forced object**.
  Seven natural spellings that might have added the same weight -- pre-increment
  in the condition, a compound loop condition, subscripting, counting down --
  are all flat at 24 or change the size.
- **Statement order merges a web's occurrences, and that is worth 9 more.**
  `CDX_FORCE=p1:w119=c9,p1:w124=c8` was worth 11 on its own. Those two are the
  literal 2 (totalsave 1.0, nocs 2, save 0.5) and the address of
  gOverlay34ActiveCount (totalsave 3.0, nocs 5, save 0.6). Moving
  `byte12 = 2` one statement earlier, ahead of `short16 = height`, takes the
  literal's nocs from 2 to 1 at unchanged totalsave, so its save becomes 1.0 and
  it outranks the address web. 17 -> 8.

**nocs is per block, not per occurrence.** The probe added a def and a use and
left nocs at 2; the statement move changed nocs without changing totalsave. That
is the lever the rest of this function needs and it is the one worth carrying to
siblings.

**What is left, with its price.** `CDX_FORCE=p1:w45=c3,p1:w51=c2` on the current
baseline scores **4**, so the four words at +0x88..+0xA4 are one more ranking:
the resource-pointer copy web (totalsave 3.0, nocs 1, save 3.0) is decided
before the height web (totalsave 4.0, nocs 2, save 2.0), and the shipped code
decides them the other way round. The height web needs the same nocs merge the
literal 2 got -- it needs nocs 1, which gives save 4.0. Not found yet: a
380-cell exhaustive single-statement-move hill climb over the twenty-statement
store block is at a local optimum of 8, and eleven width/height spellings,
declaration orders, line joins and probes are flat or worse.

The other two open words are the cost of the copy device: `record = current + 1`
makes the increment read `current`, where the shipped code reads `record`, so
`+0x54` and `+0x5C` disagree. Reverting to `record++` scores 9 and loses the
delay-slot fill. The wanted form keeps the copy live *and* increments from
`record`; five loop spellings for it, including a discarded read of `current`
and a self-assignment of `record`, are all flat at 9.

**One inherited claim is refuted.** The in-source note said the target reads
gOverlay34ActiveCount, gOverlay34Pointers and gOverlay34Count as offsets 0, 4
and 8 of one relocated symbol, "so the three were members of a single struct in
the original". The displacements are real and visible in the relocation table --
the shipped code reads the count at +8 and the pointer table at +4 of a base
whose low half is zero, and it carries six relocations against this candidate's
twelve. But writing them as one C struct is **eight bytes short**: IDO reuses
the single base register, while the shipped code materialises a fresh base for
each access. They are separate symbols at adjacent addresses, and splat names
only the first. Do not spend another pass on the struct.

#### Epoch 15 linked near-shape pass (2026-09-04)

- assignment base: `58fac8f8`. Fresh configured preflight reproduces the
  retained 125-word body, 30 relocation-masked and 32 raw positional
  differences, first mismatch `+0x0`, and frame `0x28` versus target `0x30`.
  All 12 candidate and target relocation records still agree by offset, type,
  effective identity, and addend.
- a fresh linked promotion trial classifies the retained C as `text-differs`,
  with 30 in-range words and zero outside the owned function. This is not a
  match; exact linked overlay and ROM evidence still comes from the assembly
  fallback.
- workbench diagnosis reports that the target colors one more integer pool
  web and owns one more eight-byte frame quantum. The one authorized
  structural reshape declared the semantically natural resource result local
  and used it for the candidate store, null test, and two dimension loads.
  IDO folds that local instead of allocating a home: the function becomes 124
  words in the same `0x28` frame, with 109 masked and 110 raw positional
  differences.
- the reshaped linked trial is `text-differs` with 109 in-range and 1,222
  outside words; only 5/12 relocation sites remain aligned. The original C is
  restored. This mechanism does not explain the target's extra web or frame
  home and is now exhausted; resume only with direct new evidence for the
  missing producer/home identity.

#### Whole-itable loop-current reproof (2026-08-31)

- assignment base: `d63836a7`; configured V0 reproduces 500 bytes / 125
  words, 95/125 relocation-aware positional words, 32 raw and 30 masked
  differences, first mismatch `+0x0`, frame `0x28` versus target `0x30`, and
  the previously authenticated 12 relocation identities.
- IDO 5.3 `uopt.c` was regenerated from static-recompiler revision
  `9c242adc` at the pinned `b0058f15...` source digest, instrumented with the
  shipped global-color profile and preserved whole-itable patch, and built in
  a lane-local toolchain copy. With tracing enabled, `.text`, `.data`,
  `.rodata`, relocations, and symbols are identical to stock output.
- the baseline ladder contains eight slots, including six automatic homes.
  It confirms that `record` and `current` already remain distinct frontend
  homes; the missing frame quantum is therefore not explained by the source
  having collapsed those declarations.
- the single authorized source form changed the active test from
  `record->active` to the just-assigned `current->active`. It is byte-identical
  in `.text`, retains 125 words and frame `0x28`, and leaves all comparison
  metrics unchanged. The compiler proves the two expressions equivalent
  before the missing allocation decision.
- no second ordering form was attempted because the first form produced no
  strict frame or word gain. Preserve the retained source and reopen only
  when a producer trace identifies the missing automatic home or delayed-bound
  carrier directly; do not repeat flags, loop spellings, or broad permutation.
#### 2026-09-12, lane p23-lastmile3: exhaustive colour floor

Fresh residual map: 117 byte-exact, 6 naming, 0 immediate, 2 structural; one
window and one a0/v1 cycle. The every-colour footprint sampled 106 accepted
probes over 13 coloured webs. The only winner was w45=c3 at 4 masked words;
w51=c2 was not a legal accepted colour. The two-force lattice therefore floors
at 4 with w45=c3. The source question remains the height web's missing nocs
merge and the two-word record-copy shape; no source edit was adopted.

#### 2026-09-12, lane p24-recipe: indexed scan removes two naming rows

The stock baseline reproduced 8 masked and 10 raw differences at 125 words,
size delta zero and frame 0x30. The retained indexed scan computes the record
from the array and index, keeps a current-record alias, and advances the index
only on the continuing path. The index is unobservable after loop exit. This
removes the old explicit cursor advance and the old index or-with-zero probe.

Aligned buckets improve from 117 exact, 6 naming, 0 immediate, 2 structural
to 119 exact, 4 naming, 0 immediate, 2 structural. The window at +0x40 loses
both naming rows; every other window is unchanged. The result is 6 masked,
8 raw, 500 bytes, delta zero, frame 0x30, first masked mismatch +0x88, with
12 candidate relocations. It remains NON_MATCHING; no new matched credit.

The recipe works on the loop, but deleting both pointer declarations loses
one frame quantum: the single-current form is 16 masked with frame 0x28.
Retaining the source's record-to-current alias recovers that frame without
an unused declaration. Inlining height at its stores instead creates repeated
loads across potentially aliasing record stores and costs 24 bytes. A named
resource pointer removes a load and costs four bytes of target geometry.

The retained shape's fresh exhaustive landscape samples 106 alternatives
over 13 coloured webs. Its sole winner is p1:w44=c3, scoring 2 masked at
delta zero. The source-derived resource copy web has save 3, nocs 1, and
wins v1 before the height web, whose save is 2, nocs 2. Forcing the copy to
a0 fixes four naming rows but leaves the two store-order rows. This force is
only diagnostic, never part of the candidate or build.

The named source question is how to obtain the height/resource ordering while
emitting short16 before byte12, without losing the literal-2 priority which
the earlier pass established. Three final source forms did not improve the
retained six-word residual: grouping the two stores on one physical line is
flat, carrying unscaled height gives 49 masked at delta zero, and defining
height before width gives 92 masked at delta minus four. These forms establish
no new target identity and do not explain the remaining priority, so the pass
stops under ADR 0018. All candidate sources, objects and aligned per-window
comparisons are retained privately under build/p24/overlay34CreateRecord;
the final landscape is under build/p24/record-final-landscape.

#### 2026-09-13, lane d1: emission and store-order differential

The fresh configured baseline reproduces 500 bytes, 125 words, frame 0x30,
six masked and eight raw differences, first masked +0x88. Aligned buckets
are 119 exact, four naming, zero immediate and two structural, with no gaps.
The two residual windows contain the four resource/height naming rows and
the two store-order rows respectively. There are 12 candidate ELF relocation
records versus six in the extracted target; no exact relocation claim is made.
The existing exhaustive colour landscape was read, with no repeated sweep.

The baseline census has 28 draws and 213 emission records. Each dimension
line contributes one draw. The two contested stores draw no register and
contribute two emission records apiece. Three single-change probes were kept
as ignored evidence, then the guarded baseline was restored:

- Swap the adjacent byte12 and short16 stores on this indexed source shape.
  The per-line draw/emission counts and draw-register sequence remain equal.
  The store-op identities exchange lines, which those aggregate counts do not
  capture. The structural pair disappears, but the aligned result regresses
  to 15 naming rows across five windows. The earlier literal-priority penalty
  therefore persists on the current shape; this is not a retained improvement.
- Move the height definition into a comma expression in the first byte store.
  It still evaluates after width and before any candidate store. Its one draw
  moves from the dimension line to the first-store line; the whole sequence
  stays unchanged. The total emission-record count drops by one, but all 500
  owned bytes are identical to baseline. Moving the source attribution did
  not change the effective resource/height ordering.
- Consume the resource assignment directly in its null test. The call and
  store retain their semantics. Ten emission records leave the assignment
  line, while the condition line grows from six to fifteen; the missing record
  is a source-location directive. The dimension draws and all 500 owned bytes
  remain identical to baseline. This context change does not move the blocker.

Every probe passes full-TU stock/instrumented text, data, rodata, symbol and
relocation fidelity. The three record probes neither improve the best aligned
residual nor establish a new target identity or untested mechanism. The first
reproduces an already-known order tradeoff, and the last two only move source
attribution. This is the ADR 0018 stall; no further spelling lattice was run.

The precise remaining question is unchanged: obtain the height/resource
ordering while emitting short16 before byte12 and preserving literal-2
priority. Per-line census counts alone do not expose exchanged store kinds or
operation ownership within one line; inspect the retained trace and aligned
object delta before reading an unchanged-count footer as unchanged code.

Sources, objects, scores, profiles, raw traces and aligned per-window deltas
are retained privately under build/d1/overlay34CreateRecord. Commands include
draw_census.py --save/--compare, residual_map.py --object/--against,
align_symbol.py, frame_census.py, the workbench fidelity gate and
finalize_plateau.py. No executable-byte credit or canonical C promotion.

#### 2026-09-16, lane nx-b: the block table, measured as a budget, 6 -> 2

Baseline reproduced at 6 masked (8 raw), delta 0, aligned 119/4/0/2, first
+0x88. Six batches, 37 cells; the block boundary lm-a asked for was found by
instrument, not by sweep.

**Where the boundary is and what puts it there.** The instrumented uopt's
per-web block sets on this TU and on a synthetic copy of the then-block
(private `bbprobe.py` over the lineage trace) locate block 10's end after
`byte12 = 2` (line 124) and, varied, give the rule: uopt closes a
straight-line block after a fixed budget of local-variable loads. A store
of zero or of a hoisted constant costs one (the base pointer), a store of a
variable costs two, a dimension definition costs one; the budget from the
resource test is twenty. Measured: with zero stores the seventeenth store
is the last in the block, with variable stores the ninth, with hoisted
constants the seventeenth, alternating the twelfth, and an extra definition
statement moves each by one. The o034 sequence spends its twenty exactly at
byte12 in the p9 order, and one earlier in the natural order, which is why
moving byte12 ahead of short16 gave the literal 2 its `nocs` 1 and why the
same move costs the store pair.

**What closed four rows.** Height's def and first store are in block 10 and
its two later stores in block 11 (save 4 over `nocs` 2), the resource copy
has one block (3 over 1), so the copy takes v1 first. Two self-defining
self-defining or-with-zero reads of height after byte12 add a def and a use each in block
11 at zero width (8 over 2, decided before the copy): 6 -> 2, first +0xE8,
delta 0, unforced. One such read ties and loses (6); discarded or-with-zero
reads in either block are dropped by uopt (6); three reads change the shape
(92, -4). A region opener after the dimensions with the height definition
moved to its first store also flips the pair but places the height load in
the second block (16); every other opener position is -4.

**What the last two rows are.** The ROM emits short16 before byte12 with the
literal 2 still at `nocs` 1, so its block 10 held both, i.e. at least
twenty-two loads under this budget; ours holds twenty. Folding the two
stores onto one physical line in either order is inert or worse (2, 12), the
natural order is 11, folding either or both dimension definitions into their
first stores is 27-28 with the frame moved, `register` on the base pointer
or the dimensions is inert (11, 2), an unscaled carrier for both dimensions
drops the copy (-4), height alone unscaled makes its shifted value a pool
web (34). Refuted as the boundary's cause: statement count (chaining two
zero stores moves the boundary earlier, not later), a fixed count from the
block start (an opener after byte03 leaves the rest one block).

Next: the source question is the budget's unit, since the ROM's block holds
more stores than twenty loads allow. Candidates the ROM could have used and
this lane did not measure: the stores through a base that is not a local
symbol load (the loop's `current` alias is a symbol too, but a base carried
in a uopt temp, such as the call result itself, may be free), or dimension
values that are uopt temps rather than declared symbols in a form that keeps
the copy. Price any candidate by the synthetic probe first; it is a
one-second compile per cell and reads the block sets directly.

#### 2026-09-16, lane w1-b: the budget is calibrated exactly; the pair is two loads short, six cycles

Baseline reproduced at 2 masked (4 raw), delta 0, aligned 123/0/0/2, the
byte12/short16 pair. Not moved. What the six cycles bought is the rule
itself, measured on this TU with a sliding sentinel (`byte03 = 2`, K zero
stores, `byte12 = 2`; the literal's block set read off the instrumented
uopt), so the next lane can price a form without compiling it:

- uopt closes a straight-line block when the count of LOADS OF LOCAL
  VARIABLES since the block's start reaches twenty. A store of a constant
  costs one (its base pointer), a store of a local two, `p->f = q->g` two,
  a definition statement one per local it reads however long its ILOD
  chain (`width = (candidate->resource->width - 1) << 5` is one), and a
  self-defining an or-with-zero self-defining read one. `register` changes nothing.
- Global loads cost nothing: `g = 7` and `g++` are zero units, and
  `p->f = g` is one. ILODs, constants, arithmetic nodes and the statement
  itself cost nothing. The counter restarts at the block head, not at the
  call: probes, extra stores and zero-cost statements in the call block
  before the resource test move nothing.
- Counted after propagation: `height = width - 1` and `height = width`
  fold the def away and shave two.

In the ROM's store order the block holds 19 units before `short16`, so
`short16` (two) closes it and `byte12` starts the next; both in one block
needs at most 17 before `short16`. Dropping two zero stores does it and
dropping one does not (control). The only source form measured that
reaches 17 without a declared dimension is the forwarding shape (store the
expressions once and re-read the fields), and uopt refuses to forward an
s32 truncated into an s16 field: it reloads with `lh`, 97 at plus four,
frame 0x28. Next: a form with at most 17 local loads before `short16` that
keeps two declared s32 dimensions (the frame) -- there is no such form in
the statement-per-store family, so it is either a different base symbol for
the second half (a temp, which the `candidate->resource` copy web shows
uopt will carry) or a store family cfe emits with one base load for two
fields.
<!-- plateau-handoff:overlay34CreateRecord:end -->
