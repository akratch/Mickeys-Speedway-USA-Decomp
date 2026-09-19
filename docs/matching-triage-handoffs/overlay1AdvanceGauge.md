<!-- plateau-handoff:overlay1AdvanceGauge:start -->
### `overlay1AdvanceGauge` plateau handoff

- source: `src/overlays/o001/overlay_001_middle.c`
- score: 0 differing words
- frame: 0x40
- relocations: 3
- first mismatch: none
- summary: MATCHED. Indexed `objects[index]` plus hoisted `index = count - 1` removes the extra pool web; no allocator force in the build.


#### c2-o001 diagnostic: 18 of the 25 words are pool/ring population, and that is now measured

Under ADR 0017 an isolated inert-diagnostic sweep was run against the retained
25-word candidate to size the mechanism rather than to improve the score. The
diagnostics are NOT adopted -- the cleanup queue's policy and ADR 0017 both
forbid adopting an inert form into a nonexact body -- but the number they
produce is the useful result, because it says how much of the residual is
allocation population and how much is something else.

Five zero-footprint reads placed in the loop take the candidate from 25 words
to 7 at 42 of 42 instructions, an exact 0x40 frame and delta 0 throughout:
a global read after the object dereference, a second one after the state
dereference, a count read before the loop latch, an index read between the
latch and the cursor decrement, and a loop-value read at the top of the
guarded block. Every one of the five is load-bearing -- all 31 subsets were
measured and dropping any single probe costs between 2 and 18 words -- so this
is one accumulating population effect, not five independent lucky hits.

What the diagnostic buys, in order: the three long-lived constants (the
multiplier and the two clamp bounds) move off the pool and onto the ring and
the import's address web lands on the target's colour (25 to 19), then the
loop-index and object colours align (19 to 13, then 18), then the clamp
arithmetic and the state pointer (13 to 9 to 7).

The 7-word floor is a clean three-value class swap and nothing else: the
target gives the reloaded count a pool colour and takes the scaled-index and
the import's value from the ring, and the candidate takes the count from the
ring and spends the pool colour on the import's value instead. That floor is
firm against everything tried around it -- 375 further declaration
arrangements at the diagnostic base including a spare local at all eight
positions and the volatile pad at all seven; 268 further probe additions and
substitutions across seven placement slots and 39 expressions; every carrier
form for the count through the two existing integer locals; and 36 multi-read
count-priority forms, which is lever 9's dial and it does not turn here.

The bar for the next lane is therefore precise: find the single structural
difference that puts the reloaded count in a pool colour and the import's
value in the ring. Eighteen of the twenty-five words follow from it, and the
remaining seven are that difference itself.

#### 2026-09-12 lane p19-laws re-test

The configured baseline remains 25 masked words out of 42, with exact size,
frame 0x40, and three relocations. A legal statement-order climb covered 33
compiles and reached a fixed point at 25. Address-form reloads for `D_0`, an
unsigned spelling of the multiplier literal, and deletion of the loop-value
carrier did not improve the candidate; the carrier deletion introduced two
structural rows and scored 26.

The instrumented procedure is ordinal 5. `CDX_FORCE=p1:w40=c5` was accepted
with `forced=5` and scored 13 masked words directly against the target object;
the same web forced to c2 scored 23. This prices the allocator decision but
does not identify an admissible source spelling. The candidate remains
guarded and no diagnostic force is promoted.

#### 2026-09-19 lane w25-o1gauge: L160 indexed scan is exact

Identity-gated instrumented IDO `.text` matches stock. `CDX_PROC=5` (12 p1
decisions, 12 coloured webs, no p2). `--every-colour` (105 probes, size 0)
floors at 13 with `p1:w40=c5` (loopValue v0 to a2); holding that force and
adding `p1:w21=c2` plus `p1:w11=c5` reaches 6 at delta 0. Splits of the
constant webs and of loopValue all regress. Colour therefore cannot close it.

The extra pool web was the walking cursor. Writing `objects[index]`, hoisting
`index = count - 1` above the guard, and using `loopValue = index--` -- the
same shape as matched `overlay1FindNextAngle` in this TU -- is 42/42 words,
frame 0x40, three relocations, and no pad. The walking sibling
`overlay1AdvanceObjectGauges` keeps its cursor because the inner call changes
occupancy; that form is not the lever here.
<!-- plateau-handoff:overlay1AdvanceGauge:end -->
