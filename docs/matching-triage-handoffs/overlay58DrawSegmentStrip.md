<!-- plateau-handoff:overlay58DrawSegmentStrip:start -->
### `overlay58DrawSegmentStrip` plateau handoff

- source: `src/overlays/o058/overlay58DrawSegmentStrip.c`
- score: 68/201 words
- frame: 0x88
- relocations: 8
- first mismatch: +0xF0
- summary: L100 cannot attach to the 0xFF constant. Unforced v1 needs islda save below 53.33 without a competing isop. Integer-cast split hits v1 at 73.
- assignment base: `9c62568e60deef2170043478e9473e5b42fd27b4`
- owned range: overlay 58 `+0x4C04..+0x4F28`, 804 bytes / 201 words
- fresh baseline: 99 of 201 words exact, 102 positional differences, first mismatch `+0xC`; exact `0x88` frame
- relocation proof: target and candidate each emit eight records with 8/8 offsets and types aligned; all eight candidate static identities remain unresolved, so preflight fails closed
- new mechanism evidence: the later exact Overlay 22 result rotates independent floating-point terms and uses one explicit `(*pointer).field` operand spelling; the Overlay 41 learning independently proves term rotation and commutative scalar-multiply order as valid IDO levers
- bounded probes: rotating the distance sum from `dx, dy, dz` to `dy, dz, dx` regresses to 91 of 201 words and moves the first mismatch to `+0x4`; reversing both perpendicular scalar multiplies is byte-flat at 99 of 201; an explicit dereference at the first floating-point-derived vertex store is also byte-flat
- exhausted prior work: ten cursor, alias, color-carrier, declaration/lifetime, scalar-carrier, and command-store forms plus all 119 flag combinations remain closed; no permutation was run because none of the three new probes produced a strict gain
- retained source: the original negative-index cursor spelling remains best and fixes one dead-pointer advance immediate without changing write order or semantics
- blocker: workbench still reports broad allocation mismatch, with the global-cursor/color and floating-point pools cyclically recolored; relocation identities also require resolution before any future promotion
- fresh baseline buckets: 133 byte-exact, 60 register naming, 0 immediate-only, 10 structural; size delta 0 and the same four buckets held after every retained source probe
- allocator evidence: the procedure is p1-only; its integer color table is c1 v0 through c12 t5, c13 unnamed, and c14 through c22 s0 through s8. The t6 through t9 residuals are ring temporaries and cannot be reached by declarations.
- causal force: forcing p1 web 225 to c3 a0 automatically put web 226 at c2 v1 and reduced the masked score from 68 to 37 at size delta 0. This proves the target swap is legal in the web graph, but no source form tested induced that save-ratio ordering.
- attempts 1 through 5: the L59 store-line fold and named color carrier were flat at 68; pointer and OR-zero ring probes regressed to 128, 107, and 143 masked words. All regressions were reverted.
- next action: do not reopen the prior ten forms, flag lattice, or these three FP-order probes; require a new source-authentic lever for the global-cursor/color web or whole floating-point pool
- donor check: no credible JFG use location exists; the nearest cross-title skeleton remains Conker at 0.048, with no exact coddog window or source analogue
#### 2026-09-13, lane h1: redundant step carrier differential

The fresh configured baseline reproduces 201 words, 804 bytes, frame 0x88,
68 raw/masked differences and first +0xF0. All 17 observed stack slots and
their traffic agree. Paired aligned buckets are 133 exact, 60 naming, zero
immediate and six structural rows, plus two candidate-only and two target-only
words. Static relocation counts are eight candidate and four extracted target
records, with no matching symbol tuples; runtime identities remain unresolved.
The census records 86 draws and 303 emission records. Full-TU stock/traced
section, symbol and relocation fidelity passes.

The source and shard's carrier, step-placement, order and force receipts were
read. One source probe uses stripStep directly in the loop tail instead of
currentStep, whose value is the same unchanged local. It tests whether the
redundant carrier retains a distinct scheduling or allocation identity.
Every line's draw/emission counts, the complete draw sequence, owned object
bytes, and all aligned windows are identical to baseline. The compiler already
folds this use; direct tail spelling provides no new schedule lever.

Restore the original guarded body and stop early under ADR 0018 with the
existing carrier/order negatives. This closes the tested redundant-use route,
not every possible source form. No colour sweep was repeated and no bytes are
credited. Source/object pairs, census profiles, fidelity receipts and aligned
deltas remain ignored under build/h1/overlay58DrawSegmentStrip. Commands:
configured compilation, draw_census.py --save/--compare, residual_map.py
--object/--against, workbench fidelity, finalize_plateau.py and tools/gates.sh.


#### 2026-09-13, lane `j1`: current schedule confirmation

The configured baseline independently reproduces 201 words, delta zero,
frame 0x88, masked 68 and first +0xF0. Alignment has 133 exact, sixty naming,
zero immediate and ten structural rows including two candidate-only and two
target-only words. Procedure 0 has 86 draws and 303 emissions; stock/traced
full-TU text agrees. Static tables contain eight candidate and four extracted
target records with no matching tuples; runtime binding is a separate proof.

The shard's global-cursor/colour force, floating-point ordering, named-carrier,
store-line and redundant-step controls were read alongside the source. The
latest direct stripStep control already measures identical per-line counts,
sequence and object bytes. No new source hypothesis was identified outside
those measured inputs. ADR 0018 therefore permits an early evidence stop with
zero new material attempts, rather than rerunning a closed colour landscape
or claiming that a fresh baseline itself exhausted another axis.

The guarded source is retained. The global-cursor/colour ordering and floating
pool schedule remain open; this pass adds authenticated current evidence only.
Private source, stock/traced objects, census and residual map are preserved.
Commands: assignment gate, configured compilation, draw_census.py baseline,
residual_map.py --object, finalize_plateau.py and tools/gates.sh. The full-ROM
proof covers the assembly fallback and awards no new executable bytes.

#### 2026-09-18, lane w15-o058s: colour landscape, L159 packing, L160 closed

Configured baseline still 201 words, delta 0, frame 0x88, masked 68, first
+0xF0. Aligned buckets 133 exact, 60 naming, 0 immediate, 10 structural,
with two candidate-only words at +0x1A4 and two target-only words at +0x198.
Frame ladder is 17 of 17 shared slots. Draw census is 86 draws and 303
emissions. Identity-gate: stock IDO versus instrumented IDO passed on
procedure 0 (31 p1 decisions, no p2). CDX_PROC is 0.

web_footprint --every-colour ran 145 same-kind probes over 31 coloured webs.
Ten forces beat 68 at delta 0. The unique best is p1:w225=c3 (v1 to a0) at
37, saving 31, which is the inherited v1/a0 swap. All other winners are
rivals of that radius (w62=c2, w89=c2, and the other w225 colours). L159
packing is that single force, predicted 37. No force scored 0, so L160's
delete-the-declaration path is not available.

The forced-37 object keeps size and frame. Its remaining 29 naming rows are
the t4-t9 ring cycle (not declaration-reachable). The two-word insertion
pair at +0x198 versus +0x1A4 is unchanged, so colour cannot close the
function.

Generated-index and carrier mutations, all reverted:

- Sibling-style vertices[1,2,3] plus vertices[-3]: byte-flat at 68.
- L97 if (vertices != 0) probes around colours: byte-flat at 68.
- Delete vertices and subscript the global with constants 0..3: 152 masked,
  size +56. IDO rematerialises the base; there is no induction index.
- Indexed colours and coordinates through vertices, assign cursor = vertices
  + 4: 193 masked, size -72. Homes move.
- Walk vertices for coordinates and store the cursor at the end: 194 masked,
  size -48. The target walks the global.
- do-while-0 around colour stores: 93 masked at delta 0.
- do-while-0 around the two packet appends: 201 masked, size +20.
- Reuse y as the 0xFF colour carrier: byte-flat at 68; copy-propagated away.

Retained source is still the negative-index walking cursor. Do not retry
dx,dy,dz rotation, perpendicular muls, explicit dereference, the ten prior
cursor forms, the 119-flag lattice, or these generated-index / L97 cells.
The named remaining variable is how to make web 226 (the 0xFF constant,
save 160/3, currently a0) outrank web 225 (expression, save 230/3, currently
v1) from source, without a force. Landscape evidence lives outside the tree
under the lane scratch directory.

#### 2026-09-19, lane w28-o058s: L100 leftovers vs the 0xFF/v1 ranking

Configured baseline independently reproduces 201 words, delta 0, frame 0x88,
masked 68, first +0xF0. Aligned buckets 133 exact, 60 naming, 0 immediate,
10 structural, with two candidate-only words at +0x1A4 and two target-only
words at +0x198. Register census is the inherited v1/a0 swap (24+17) plus
the t6-t9-t4-t5 ring. Identity-gate: stock IDO versus instrumented IDO
.text is byte-identical (816 bytes). CDX_PROC is 0, 31 p1 decisions, no p2.

Force receipts, scored with score_symbol.py --object against the forced
object, acceptance read from forced= not from the bytes:

- p1:w225=c3 accepted (forced=3). Masked 37, delta 0. Web 226 then takes
  v1 unforced. Packed 37 is still not a match.
- p1:w226=c2 declined (forced=-2). At decision time v1 is already held by
  the islda, so 226 is forbidden c2. Directly forcing the 0xFF web onto v1
  is not a legal colour.

Decision variable, re-derived from this capture (web numbers move; identify
by type/raw): the v1 occupant is type=1 dtype=0 islda of
gOverlay58StripVertexCursorReloc, save 230/3 = 76.67, bbs 6-12. The 0xFF
web is type=2 dtype=8 raw 0xff, save 160/3 = 53.33, same blocks. p1 max-save
colours the islda first onto v1; 0xFF takes a0. Type-1 wins a save tie
because it is numbered first. L100's divisor does not reach the islda
(compiler temp) or the constant (discarded 0xFF or 0, 0xFF and -1, 0xFF xor 0
leave totalsave at 160/3).

L100 leftover probes, all reverted, 0xFF colour named:

- vertexCommand or= 0 after its assignment: delta 0, masked 68. That
  integer's save doubled 20 to 40 and moved a2 to a1. 0xFF unchanged. This
  is the leftover form that counts on an already-live integer.
- last-declared loop-local zero or= 0 (L109): delta 0, masked 68. Depth-1
  phantom was eliminated; no a0 occupant appeared.
- empty if (vertices), if (1), if (y), if (vertexCommand), if (slot): all
  regress (93 to 199) via a new region. Overlay-22 empty-if does not drop
  the islda.
- named s32 white = 0xFF with stores still 0xFF: copy-prop, 68.
- named white, stores use white, discarded white or 0: 0xFF web gone, white
  save 160/3 on a0, islda still v1, masked 87, delta 0.
- named white plus white or= 0, stores use white: save 170/3 on a0, still
  below 76.67, masked 98, delta +4. register and function-scope the same.
- L97 if (1) wrap of the colour stores: islda nocs stayed 3, masked 93.
- taking and leftover-or on a slot while still walking the global: islda
  save unchanged or raised, 68 or 93.
- y or= 0 and y and= -1 after y = (s16)y0: masked 67, delta 0, 134 exact /
  59 naming, structural unchanged. v1/a0 swap untouched. The one-word gain
  is the inner-if a-register shuffle (vertexCommand a2 to a1). A second
  leftover on y is CSE'd. Combined with an islda-split that had tied 0xFF
  at 53.33, y or= 0 restored the islda win. Do not adopt: antagonistic
  with the named lever (L88).

Unforced 0xFF-on-v1, none adopted:

- Function-scope slot = and the global, walk (*slot): 0xFF takes v1
  (islda save 40/4 = 10), size delta +68, masked 210. Proves the ranking
  arithmetic: islda save below 53.33 is sufficient.
- Integer-cast address spelling on the first 9 coordinate field stores
  (`(u32)&global`, also s32 and u32+0): 0xFF takes v1 at delta 0, masked
  73. islda 150/3 = 50 on a1, the cast isop 100/2 = 50 on a0. Aligned
  buckets 133 exact / 56 naming / 1 immediate / 15 structural, extra
  insertion pair, first mismatch +0xCC. The v1/a0 swap is gone and replaced
  by a1/a0 x24 plus more structure. Worse residual than 68. char*/u8*/void*
  and *& spellings CSE back to one islda, 68, no split. Eight-arrow integer
  cast ties 0xFF at 53.33; type-1 keeps v1. Duplicate 0xFF store does not
  raise totalsave.

Stall: leftover forms that attach at delta 0 do not move the 0xFF/islda
ratio (constant probes do not count; named carriers copy-prop or score
87+; empty-if opens a region). The only delta-0 unforced v1 is the
integer-cast islda split at 73 with a competing isop on a0. Three
gentler address spellings after that HIT produced no new ranking and
CSE'd to 68. Retained source is unchanged. Next: a split spelling of
the global's address that does not CSE, does not birth an isop with
save above 53.33, and keeps the original addressing mode; or a live
integer leftover that is the 0xFF value without emitting a new `li`.
Do not retry FP order, the prior ten cursor forms, packed 37, named
white carriers, discarded 0xFF probes, L109 zero at depth 1, or y or= 0
as a claimed step toward v1.
<!-- plateau-handoff:overlay58DrawSegmentStrip:end -->
