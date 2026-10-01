<!-- plateau-handoff:overlay101DrawPanel:start -->
### `overlay101DrawPanel` plateau handoff

- source: `src/overlays/o101/overlay101DrawPanel.c`
- score: 0/268 words, promoted
- frame: 0x178
- relocations: 17
- first mismatch: none
- summary: Matched. Plain locals, edges written as expressions, and the record pointer assigned before the colour call; no work struct, volatile or region.

#### 2026-10-01, lane b-o101: matched by rewriting the inherited shape

Promoted at 268 of 268 words, delta 0, frame 0x178, 17 of 17 relocations.
Nothing in the retained candidate was tuned; the body was rewritten from the
target listing and four inherited artefacts were dropped together.

The decisive one is where the record pointer gets its value. With the pointer
assigned after the intensity-colour call, uopt forwards the array address into
the first three records' stores and proves they miss the address-taken
colours, so the first colour is read once for three records, two words short.
Measured with every record storing one colour: eight loads for ten records.
With the pointer assigned before the call, or at its declaration, every store
address is still folded to a frame offset but the stores are no longer proved
disjoint from the colours: ten loads for ten records, one per record, and the
last record goes through the pointer's own register. That is the reload the
earlier lanes produced with volatile members and the last-record register
they produced with a region boundary.

The rest follows from plain declarations. x, y, width and height are ordinary
locals shared by both drawing arms, which gives the mode 1 and 3 height its
coloured register (the ring question recorded on 2026-09-10 and 2026-09-11).
The right and bottom edges are written as sums at each use; uopt commons them
and spills the one that crosses calls to a compiler temporary, which is the
declared-scalar deficit the 2026-09-11 frame reading named. Declaring the
pointer between the colours and the array supplies the four-byte cell above
the records; declared after the array it sits below it and shifts the three
temporaries by four bytes, eight words.

Path, each measured at the configured flags: natural locals with the pointer
assigned after the call 200 words and eight bytes short; the same with the
pointer initialised at its declaration 47 at delta 0, all of it the array
offset; a pad above the array 8; the pointer declared above the array 0.
Per-record do-while or if regions are twelve bytes long and unrelated, and a
compound pointer advance is eight bytes long.

Closures this breaks: the declared-symbol-count decision variable was right
about the frame and wrong that no source form reaches it, because every form
tried kept the work struct. The overlay101BuildBorder comment that only a
region boundary or volatile separates two colour reads is a statement about
its inherited shape too; that function is already matched and was not
re-derived here.

Gates: gmake verify, check-overlay-syms and promotion-proof pass.


#### 2026-09-11, lane p2-close: the block is SMALLER, not larger, and the older reading inverts

Baseline reproduces: 1,072 bytes, 268 of 268 words, delta 0, masked 36, first
mismatch +0xB4. Aligner: 234 byte-exact, 16 register naming, 8 immediate-only,
13 really different.

**The eight immediate-only words are three spill displacements, and naming them
settles the direction.** `frame_census` puts the target's spills at +0x48 with
one store and three loads, and at +0x5C and +0x60 with one store and one load
each; this candidate's are at +0x5C with one store and three loads, and at
+0x50 and +0x54. Read against the object they are the same three values in the
same pattern: `x + work.width` carries the three loads, `y + 2` and `x + 14`
carry one each. Only the displacements differ, which is exactly eight words.

**Therefore the target's declared block ends at +0x64 and is 276 bytes, where
this candidate's ends at +0x58 and is 288.** Compiler temps sit immediately
below the declared block -- confirmed here by adding padding declarations and
watching the temp slots move with the block bottom -- so the target touching
+0x60 puts its block bottom no lower than +0x64. `work` is pinned to +0x64 on
both sides by the four addresses passed to BuildIntensityColors, so the target
declares three fewer four-byte objects than this candidate, and by
`frame = round8(56 + temps + block)` its compiler-temp pool is eleven cells
against this candidate's eight.

**That inverts the older record.** "A seventh declared local would supply the
missing ring web but moves the frame" is measuring the right symptom in the
wrong direction: the shipped source declares fewer homed objects, not more, and
three of this candidate's declarations are compiler temporaries there.

**The ring census says the same thing from the other end.** The instrumented
ugen's caller-saved free list for this procedure is only t6, t7, t8 and t9:
globalcolor takes v0, v1 and t0 through t5, and ugen strikes those from the
pool at procedure entry. The shipped code spends t1 as an expression temporary
in the mode-1/3 arm, so it runs a wider ring, which means one fewer p1 web.
One fewer p1 web and three fewer declared scalars are the same fact. The
earlier "t1 twelve times against four" census was right and its reading --
"one more web in the expression ring" -- was backwards.

**Measured this pass, every cell at delta 0 unless stated.** One through five
unused pointer declarations after `bottom` give frames 0x180, 0x180, 0x188,
0x188 and 0x190, so the temp pool is rigid at eight cells for this declaration
set and no padding edit reaches the target's layout. Deleting any of `x`, `y`,
`right` or `bottom` and inlining its expression costs 48 to 436 bytes, because
`&work.full` escapes and `work.width`/`work.height` therefore reload after
every call -- so L120's "a commoned load deletes at zero cost" does not apply
to this struct. Splitting `width` and `height` out of the struct into ordinary
locals and dropping the `right` and `bottom` declarations keeps size and frame
exactly, at 87 masked, and is the only form found that does. A full
reconstruction as nine separate scalars above a trailing `records[20]` array
reproduces the target's spill pattern exactly -- one three-load slot and two
one-load slots, in that order -- but lands eight bytes low, because uopt then
makes nine temporaries where the target makes eleven. Reusing `x` and `y` for
the mode-1/3 argument expressions is byte-inert. Peephole-removable no-op masks
on `panel->mode`, `panel->intensity` and `panel->alpha` all move the ring but
all regress, to 91 through 252.

**Reopen condition.** A source form that removes one declared scalar without
adding a reload, or that makes uopt create two more temporaries against a
276-byte block. The decision variable is the declared-symbol count, not a
colour and not a spelling.


#### 2026-09-11, lane p7-ovl2: the array length is pinned, and the struct-slot
#### carrier is refuted

Baseline reproduces: 1,072 bytes, 268 of 268 words, delta 0, masked 36, first
mismatch +0xB4, frame 0x178. Aligner buckets, unchanged before and after
because nothing was adopted: 234 byte-exact, 16 register naming, 8
immediate-only, 13 really different.

**L112 is closed on this function.** The decision variable asks for a declared
block twelve bytes smaller, and the record block in the work struct is the one
unobservable length in the source, so it is the obvious free parameter. It is
not free. Sweeping the record count from 14 to 24 gives frames 0x130, 0x140,
0x148, 0x158, 0x160, 0x170, 0x178, 0x188, 0x190, 0x1A0 and 0x1A8 in order, and
only the incumbent 20 holds the shipped 0x178. Every other count scores 46 or
92 with the first mismatch at word 0. So the twelve bytes cannot come from the
array, and the frame identity has no cell to give here.

**The struct-slot carrier is refuted too.** The one form that removes a declared
scalar without declaring anything new is to carry the two derived edges in the
work struct's own width and height members, which already have homes inside the
block and are already loaded. Measured: the bottom edge alone costs 80 bytes
and 251 masked words, the right edge alone 60 bytes and 252, both together 140
bytes and 271, with the frame falling to 0x170, 0x168 and 0x160 respectively.
The cause is the one the closure already names for inlining, now measured for
the reuse form as well: the work struct's address escapes through the four
pointers handed to the intensity-colour builder, so every write to a member is a
store and every later read is a reload.

**So the decision variable stands, with two more routes closed.** A source form
that drops one declared four-byte scalar at zero byte cost, where the value does
not live in the escaping struct and is not re-derived from it. The array length,
the struct-member carrier, the declaration orders, the qualifier and type
lattices, the padding declarations, the nested-assignment carrier and the region
spans are all recorded as measured and negative.

#### 2026-09-12, lane p19-reopen: address reloads and new-shape order tested

Assignment was base-only. The retained baseline remains 268 words, delta zero,
frame 0x178, first +0xB4, 36 relocation-masked differences. Buckets before
and after are 234 exact / 16 naming / 8 immediate / 13 really different.
The alignment includes inserted/deleted rows and displacement tax -1; these
buckets are not a partition of 268 positional words. No candidate body change is adopted.

The configured full-TU instrument passes byte identity, including an external
source-path control, and procindex ordinal 0 emits p1 records. The frame
census reproduces the inherited declared-home versus compiler-temp deficit.
Forcing web 95 to split is explicitly accepted (forced=-1), adds 12 bytes and
regresses to 218 masked differences; this is not evidence for a free ring
slot. The old single-web force closure was not treated as an order proof.

L144 is a distinct axis from deleting volatile. Replacing the dim read with
an ordinary address-form load gives 37 masked, buckets 238/14/8/12; changing
dimmer gives 38, buckets 234/16/8/15; changing both gives 37, buckets
239/14/8/10. All retain size. Thus the old volatile-only negative did not
retire this lever, but the measured address forms do not improve the retained
score. A dependency-guarded order climb on the both-address shape evaluates
39 forms in one pass and remains 37.

Separating width/height from the escaping work struct and inlining the two
edge values reproduces 87 masked, delta zero, buckets 213/34/4/22. The
inherited order sweep did not cover this shape: a fresh guarded climb tests
35 forms in one pass and remains 87. Replacing the record pointer with an
index gives 217 masked and removes eight bytes; combining that change with
the split edges removes twelve bytes and still scores 217. Neither is a
same-size improvement. The final new-shape order and carrier-removal controls
supply no improved residual or new decision mechanism, so this packet stops.

The declared-home/temporary-pool diagnosis survives. A next packet needs a
source form removing the redundant homes without alias-sensitive reloads,
and must re-price the frame and ring on that shape. Existing order results
only cover their named shapes. Commands: alignment, residual map, register
and frame censuses, configured stock/instrumented compilation, direct forced
object scoring, the guarded blockclimb engine, `tools/finalize_plateau.py`
and `tools/gates.sh`. Source variants, objects, records and every climb
measurement remain in private external scratch. Eighteen relocation records
are retained; no overlay relocation promotion is claimed. Full-ROM proof
covers the assembly fallback, not these nonexact candidates.

#### 2026-09-12, lane p23-lastmile3: exhaustive colour floor

Fresh residual map: 234 byte-exact, 16 naming, 8 immediate, 7 structural,
with three candidate-only and three target-only offsets. The every-colour
footprint sampled 145 probes over 23 coloured webs; no accepted force beat 36,
so the winner list is empty and the lattice floor is 36. The named source
question remains removal of one declared scalar without an alias-sensitive
reload, or creation of two compiler temporaries at the target frame shape.

#### 2026-09-12, lane p24-second-mid: split-home and typed-edge controls

The base-only gate passes. Fresh configured baseline: 268 words, zero size
delta, frame 0x178, 36 raw and masked differences, first +0xB4. Aligned rows
are 234 exact, sixteen naming, eight immediate, seven paired structural,
and three candidate-only plus three target-only words. The object and owned
runtime table both contain seventeen relocation records; the old eighteen
was stale. This count is not a new identity proof. The prior exhaustive
colour landscape was read and not repeated.

An external source-path control is full-TU text-identical to the configured
baseline. Seven source controls then investigate the declared-home deficit.
Nine separate scalars with a trailing record array and inline edges reproduce
the expected eight-byte frame deficit: frame 0x170, same executable size,
105 masked. A region around the definitions keeps frame 0x170 but adds eight
executable bytes, at 267 masked. A region around record emission falls to
frame 0x168 and adds sixteen bytes, at 265 masked.

Giving the record edges unsigned arithmetic identity while later calls retain
the signed form reaches the target frame 0x178, but adds sixteen executable
bytes and scores 220. Thus that route can create temporary homes, but they
are not free. Isolating the right edge costs eight bytes at 188; isolating
the bottom costs four at 215; applying unsigned edge identity to all uses
costs twelve at 220. Coordinate inputs and dimensions are signed halfwords,
so the original edge sums fit s32, and the unsigned spellings preserve the
stored low bits; none is adopted. Each object has an aligned comparison with
the retained baseline, and the first split-shape controls are also compared
against their own separate-scalar base. No positional improvement hides a
local regression.

The last useful finding is that typed identity can repair frame size only
with extra executable work here. The final three isolating controls neither
improve the retained residual nor establish a zero-cost temporary route;
stop on that stall. The next question remains two compiler temporaries
without extra instructions or alias-sensitive reloads, not another colour
sweep. Source and object artifacts, frame census, scores and aligned maps
are retained under build/p24/overlay101DrawPanel/. Commands: lane_status,
configured compilation, residual_map --object/--against, frame census,
finalize_plateau and tools/gates.sh. The original guarded body is retained.

<!-- plateau-handoff:overlay101DrawPanel:end -->
