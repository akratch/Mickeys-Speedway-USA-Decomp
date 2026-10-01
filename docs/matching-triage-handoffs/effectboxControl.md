<!-- plateau-handoff:effectboxControl:start -->
### `effectboxControl` plateau handoff

- source: `src/main/spranim.c`
- score: 51/193 words
- frame: 0x80
- relocations: 5
- first mismatch: +0x48
- summary: 57 to 51 by declaration placement (state spill now below the hit list). Left: hit-index spill 0x7C vs 0x6C; cursor/index a2/a3 roles swapped

Summary before this remeasure: Exhaustive landscape reaches 38 only by forcing web 60 to a2; source cursor removal regressed structurally and declaration/counter forms were byte-flat.
#### 2026-09-12 (lane `lane/p7-res2`): 60 to 57 on one named local; the two stack homes are not a declaration question

**Naming the hit's state pointer in a local inside the conditional body pays
three words.** Writing the body as `void *st = hit->state64;` followed by the
store through `st`, instead of storing through `hit->state64` directly, takes
the aligner from 133/46/10/4 to 136/43/10/4 (byte-exact / register-naming /
immediate-only / really-different) and the masked count from 60 to 57. What it
heals is the ring phase of the loop's second unrolled copy: the pointer
arithmetic that re-derives the hit cursor after the first body now draws the
same temporaries the target draws. This is L76's family read in the direction
the law's receipt gives it -- the local is not codegen-inert, it is
instruction-inert, and the allocator slot it consumes is the payment.

Five other store spellings are byte-flat against the baseline at 60: an
`SpranimEffectBox **` cast in place of `void **`, indexing the state pointer as
a word array at 0x32 rather than adding 0xC8 to a byte pointer, the same two
with the typed array, and typing the hit list as `SpranimB798Target *hits[10]`
so the cast at the read site disappears. Only the named intermediate moves.

**The two spill homes are an allocator placement, not a declaration order.**
`frame_census` reports the same frame (0x80) and the same slot count (8) on
both sides, with two cells differing: the candidate spills the plane-entry
pointer and the state pointer at +0x78 and +0x7C, and the target holds the same
two values, with the same traffic (2ld 2st and 3ld 3st), at +0x6C and +0x44 --
one above the hit array and one below it. The candidate's compiler *always*
parks its spill block immediately below the argument save area, whatever is
declared: measured across eleven hit-list lengths (6 to 20), six declaration
orders of the three inner-block locals, the inner block opened and dissolved,
the plane-entry local at function scope and at block scope, and four pad locals
of 8 and 16 bytes at function and block scope, the spill pair tracks the frame
top in every one of the 120 forms. A 16-byte pad at function scope takes the
top and pushes the spills down, so the ordering rule is "declared function-scope
locals, then spills, then inner-block locals"; the target's ladder brackets the
array, which that rule cannot produce. The next lever is therefore what makes
those two values take *declared* homes rather than spill cells, not where the
declarations sit.

**The rest is one coherent register cycle.** `register_census` reads 97% of the
substitutions following their source's dominant target over 12 source
registers, a single global mapping with two windows (the second opening at
+0x1E4), and a closed a2/a3 cycle -- one ring-phase fact by the brief's own
reading, not 43 colour problems. The per-iteration tell is that the target
reaches the hit's state pointer with one load off the hit cursor in a
branch-likely delay slot and again on the fall-through path, where the
candidate copies the cursor into the argument register first and loads through
the copy; the copy is what gives as1 an ordinary delay slot to fill, so the
candidate never emits the likely form. Removing that copy is the same question
as the a2/a3 cycle.

Four L97 `if (1)` regions -- around the hit block, around the search call and
its loop, around the unrolled body, and around the state fetch at the top -- are
byte-inert at 57. L136's lever is for a symbol-level interference and this
residual is not one.

Axes covered: 24 store/hit-list spellings; 120 declaration-order, scope, array-
length and pad forms; 11 array lengths on the pre-adoption base; four L97
regions. Not covered:
an instrumented free-list capture at the unrolled body, which is what would
turn the a2/a3 cycle from a coherent mapping into a named draw.

#### 2026-10-01, lane d-res2: 57 to 51 by declaration placement

Every local is now declared once at the top of the function (the inner-block
declarations of `entry`, the hit list, the counters, `hit` and `st` moved up;
codegen is unchanged by the move: 57). Their order is then a layout variable:
spill cells are laid out against the declared homes, and a 150 s climb over
the seven declarations found 57 -> 54 -> 51 with `entry`, `hit`, `hitCount`,
`processed`, `hits[10]`, `state`, `st` in that order. That order puts the
state-pointer spill below the hit list (target 0x44) instead of above it, so
one of the two spill-cell rows now agrees. Hit-list lengths 6 to 13 at this
order: 50 (6, size-exact but the extent is wrong), 63, 54, 60, 51, 63, 54, 63,
so ten stays. Left: the hit-index spill is at 0x7C where the target has 0x6C,
and the loop cursor/index pair takes a2/a3 in the opposite roles (the
carriers of the earlier notes were not revisited).
<!-- plateau-handoff:effectboxControl:end -->
