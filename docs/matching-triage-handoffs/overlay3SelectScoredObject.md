<!-- plateau-handoff:overlay3SelectScoredObject:start -->
### `overlay3SelectScoredObject` plateau handoff

- source: `src/overlays/o003/overlay3SelectScoredObject.c`
- score: 118/118 words
- frame: 0x80
- relocations: 5
- first mismatch: none
- summary: ROM-exact and promoted: 118 words, frame 0x80 and all 5 relocation identities; the loop subscripts the helper's array by index so strength reduction owns the cursor, and one unused pointer declaration keeps the frame.

Matched by [lm-a](../lastmile-region-boundary.md).

#### 2026-09-16, lane lm-a: matched on the generated cursor (L160); promoted

21 to 0 at delta zero, unforced, in two measured cycles; aligned
110/5/1/1 (one candidate-only, one target-only word) to 118/0/0/0; frame
0x80 and 5 relocations unchanged; `gmake verify` prints the expected SHA1
from the C and `gmake promotion-proof` passes (`identity=static`, 5/5).

- The named decision -- "what makes uopt copy the helper's return register
  into a caller-saved argument register while the cached path keeps the raw
  result" -- is L160's. With the declared `cursor` walking the array the
  helper result was one web (114: save 20, `nocs` 1, block 14 only),
  coloured v0 unopposed, and both reloads of `count` had v0 forbidden by it.
  With the loop written `object = objects[index]` and no cursor arithmetic,
  strength reduction owns the walking pointer and uopt keeps the call result
  for the cached path, copies it into a1 at the split point before the
  guard, and hands both `count` reloads v0 -- the ROM's shape, including
  `addiu s3, v0, -1` in the delay slot. One cell: 21 to 0.
- The frame is the second fact. Deleting the `cursor` declaration outright
  is 7 words with the frame at 0x78. An unused pointer-typed declaration
  (`cursor` kept, or any other `Overlay3Object **`) restores 0x80 and 0
  words; an unused `f32` beside `dx`/`dz`, before or after, is eliminated
  and stays at 7, and so is a used `distance` local for the sqrt result.
  So the ROM's body declared one more pointer than it used; `cursor` is
  kept unused and listed in `docs/cleanup-queue.md`. L99's "an unused f32
  or pointer is not eliminated" is only half right on this TU: the f32 was.
- Previous lanes' boundary/copy controls (`if (1) { }` after the call,
  after `result = 0`) are inert at 21, so the copy is a split placement
  and not a block question.

#### 2026-09-12, lane `p9-tight`: the shape is named and two forces price it

The residual reproduces at 21 masked words, size delta 0, frame 0x80, and it is
**one block**: the entry and cached path. Every instruction from `+0xA0` onward
is byte-exact, so the loop body and the scoring are finished.

**The shape, exactly.** The ROM copies the helper's return register into `$a1`
before the guard. The cached path then keeps using the raw `$v0` result, which
leaves `$v0` free for both reloads of `count` -- and the index falls straight
out of it as `addiu $s3, $v0, -1`, where this candidate needs
`addiu $v1, $a0, -1` plus a `move $s3, $v1`. The loop path reads the copy:
`addu $s5, $a1, $t0` against our `addu $s5, $t0, $v0`. So the ROM spends one
`move` at the top and saves one at the bottom, at the same 118 words.

**Records** (instrumented object `cmp`-identical to the configured one, sixteen
integer p1 decisions): `objects` is web 114 -- save 20, nocs 1, totalsave 20,
bestcost 0, `forbidden0` empty -- so it takes c1 `$v0` unopposed, fourth in
decision order. The two `count` reloads are webs 17 and 36 at save 1.5 and 1.0
and take c3 `$a0`.

**Two forces, both recorded accepted, prove the ROM's shape is neither a colour
nor a split of that one web:**

- `p1:w114=c4` (objects to `$a1`) scores **78 at delta -4**. It removes the
  `move $s3, $v1` without adding the copy, because with the web coloured `$a1`
  from the start no copy is needed.
- `p1:w114=s` (the split path) scores **79 at delta +4**.

The ROM has *both* the copy and `count` in `$v0`, which means uopt kept the
call result as its own range and handed the loop use a caller-saved copy.
That is a range decision, not a colour one.

**A source-level second pointer local is not the way in.** `base = objects`
assigned after the call, at the top of the `else`, or used for the cached path
instead all cost 8 bytes of frame (0x88) and score 30 -- and so does replacing
the unused `u16 timer` with it, so the frame is not paying for `timer` and the
pointer run cannot absorb one more declaration. A two-step
`cursor = objects; cursor = &cursor[index];` and a `cursor = objects + index;`
spelling are byte-identical to the retained form at 21.

**Decision variable:** what makes uopt copy a helper's return register into a
caller-saved argument register while leaving an earlier-block use on the raw
result. L105 is the nearest law and it points the other way -- uses in the
call's own block get the raw register and later blocks get the *callee-saved*
copy; here the copy is caller-saved and the raw use is in a later block than
the copy.

**A confound the next lane should avoid.** A 240-point declaration sweep over
the five `s32` and two `Overlay3Object **` locals read 27 to 28 everywhere,
which looks like a clean negative on declaration order and is not: it also
re-packed the declarations one-per-kind onto their own lines, while the
retained body packs several per line. That sweep measures L59 line packing as
well as L143 order. Re-run it with the packing held fixed before believing it.

#### 2026-09-12, lane p23-lastmile3: exhaustive colour floor

Fresh residual map: 110 byte-exact, 5 naming, 1 immediate, 1 structural, with
one candidate-only and one target-only offset. The every-colour footprint
sampled 104 probes over 17 coloured webs; no accepted force beat 21, so the
winner list is empty and the lattice floor is 21. The named source question
remains the caller-saved copy of the helper return before the cached-path
guard, not a colour or split of the objects web.

#### 2026-09-13, lane g1: index and helper-copy schedule controls

The base-only assignment and fresh configured baseline reproduce 472 bytes at
zero size delta, frame 0x80 and five relocations. The 21 masked positional
differences align as 110 exact, five naming, one immediate and one structural
row, plus a candidate-only offset at +0x88 and target-only offset at +0x48.
Stock and instrumented full-TU text agree. The baseline spends 28 draws and
166 emission events. The existing no-winning-colour landscape is not repeated.

Moving the index definition inside the count guard removes the candidate-only
word at +0x88 but leaves the required helper copy at +0x48 missing. Size falls
by four bytes; the aligned result has 104 exact, five naming and eight
structural rows, with one target-only word. Every draw and per-line emission
count stays unchanged: this is a range/scheduling change, not a scratch draw.
The word deletion does not qualify as an improvement of the retained residual.

The following three controls all preserve the baseline object and draw order:
seed the existing cursor before the cached-path guard and use it as the loop
base; initialize the objects declaration from the helper instead of assigning
it in the body; swap objects/cursor declarations while holding all physical
line packing fixed. The seed adds one emission event and the initializer moves
four events to its declaration line; neither is an executable copy or a new
draw. The packed declaration swap changes no census line at all. This last
control avoids the earlier declaration sweep's line-packing confound, without
claiming to exhaust all declaration permutations.

After the informative index control, these three unchanged-output controls
provide the ADR 0018 stall. The original guarded candidate is restored. The
missing decision remains a caller-saved helper-result copy that preserves raw
cached-path use while releasing the return register for the count reloads;
none of the tested forms creates it. No match or linked candidate proof is
claimed. Source snapshots, objects, traces, census and aligned maps remain
ignored under build/g1/overlay3SelectScoredObject. Commands: draw_census.py,
residual_map.py --object/--against, configured compiler, and finalize_plateau.py.

<!-- plateau-handoff:overlay3SelectScoredObject:end -->
