<!-- plateau-handoff:overlay1ConsumeNearbyPending:start -->
### `overlay1ConsumeNearbyPending` plateau handoff

- source: `src/overlays/o001/overlay_001_tail.c`
- score: 0/69 words, promoted
- frame: 0x48
- relocations: 1
- first mismatch: none
- summary: Matched. Plain counter in a while loop, a never-read store of a pointer load before the loop and at the end of the body, and an empty test of the element pointer.

#### tu2-o1tail: the countdown form is settled and is not the residual

Re-measured at the assigned base: 31 masked words, 69 of 69 instructions,
0x48 frame exact, prefix exact to row 14. The target computes count-1 before
the guard branch and stores it in the branch's delay slot, then reloads the
counter from its stack slot inside the loop, which reads as `if (count--)`.
It is not: `if (count--)` and `if (count-- != 0)` and a pre-tested
`while (count--)` each add one instruction (70) and take the residual to 51.
The current `if (count != 0) { count--; do ... while (count--); }` is the only
form of the four that holds 69 instructions.

The real residual is the block starting at +0x60, where the target reloads the
counter from sp+60 and recomputes the element address, while the candidate
keeps the object pointer live and reuses it. Next lever is that reload, not the
loop shape.


#### c2-o001: declaration order is not the pool-priority lever

The previous record's next lever was "find what makes uopt reserve those two
pool colours for `other` and `otherState`". Declaration order is not it, and
that is now measured rather than assumed: all 120 permutations of the five
locals were compiled from the retained base. None improved on 31; the retained
order and one neighbour are the joint best, three permutations reach 32, and
the rest run from 33 to 36. Do not repeat the declaration lattice.

Worth recording alongside it: `overlay1AdvanceGauge` in the middle TU carries
the identical shape at 25 words -- the target spending its pool colours on the
loop-body pointers and leaving the counter and a loop-invariant constant to
the ring, the candidate spending one extra pool colour and sliding every ring
value one position. Two functions, two TUs, one mechanism. Whatever buys or
sells that pool web will close both.


#### c2-o001 diagnostic: seven of the 31 words are pool population, and 24 are not

The previous section's next lever asked what makes uopt reserve the two low
pool colours for the loop-body pointers. An ADR 0017 inert-diagnostic sweep
answers half of it and bounds the other half. Nothing here is adopted.

198 single zero-footprint reads across eleven placement slots were measured.
The best is a state-pointer read placed between the list getter and the
counter guard, or the equivalent pair of loop-body pointer reads placed before
the radius computation: 31 words to 24, at 69 of 69 instructions and delta 0.
That is the pool web the earlier record predicted, and it is worth seven
words. A four-round beam over 735 further combinations finds nothing below 24,
so the remaining 24 words are a different mechanism and not more of the same
population effect.

This matters for `overlay1AdvanceGauge` in the middle TU as well, which the
previous section pairs with this one. There the identical diagnostic family is
worth eighteen of twenty-five words and leaves seven. So the two functions do
share the pool-population mechanism, but they do not share the residual after
it: this one keeps three quarters of its residual and that one keeps under a
third. Treat them as one lever and two different remaining causes.


#### p8-o001: the colour arrangement is source-reachable and worth 7, and the whole rest is one web that should not exist

The function issues one call, so by the call test it emits p1 records only. The
instrumented toolchain's object is byte-identical to the tree's, checked with
`cmp` on the whole object; the identity gate needs the TU's own
`-Wab,-r4300_mul` override, and a hand-written compile line without it reads 33
instead of 31 and would have attributed everything to the wrong pass.

Four webs were identified by forcing each to `s2` and reading which register
moved, which removes the guesswork the earlier records left:

- web 36, save 30.0, is `otherState`; the candidate gives it `v1`, the target
  gives it `a0`.
- web 30, save 25.0, is `other`; the candidate gives it `a0`, the target `v1`.
- web 18, save 16.5, is the counter's read-modify-write value, shared by the
  guard and the latch; the candidate gives it `v1`, the target uses `t6`.
- web 75, save 15.0, is `pending`; candidate `a0`, target `v1`.

So the target's order is other, otherState, pending, and the counter is not
coloured at all. Forcing exactly that arrangement -- `p1:w36=c3,p1:w30=c2,
p1:w75=c2` -- takes 31 to 24, and adding `p1:w18=c7` on top takes it to 16. The
colour table decodes on this procedure as c1 `v0`, c2 `v1`, c3 `a0`, c4 `a1`,
c5 `a2`, c6 `a3`, c7 `t0` through c12 `t5`, c13 unnamed, c14 `s0`. **`t6` is not
a colour**, which is the decisive fact: the target's counter register is a ugen
scratch temp and no globalcolor decision can produce it.

The colour flip is a save-ratio question and it is reachable. `other` has
totalsave 50 and `otherState` 60 at nocs 2 each, one reference apart at the
loop-depth weight of 10; one more reference to `other` ties them, and a tie
keeps the lower web number, which is `other`. One zero-footprint identity read
of `other` placed anywhere in the loop body does it: 31 to 24, at 69 of 69
instructions and delta 0, with `other`, `otherState` and `pending` then taking
`v1`, `a0`, `v1` exactly as the target does. A second zero-footprint draw -- a
u8 mask on the kind comparison, L127 -- takes 24 to 18 by putting the scratch
ring back in phase for the second half of the loop body.

Neither probe is adopted. They are artificial identity operations, the function
does not close on them, and an artificial floor in the tracked source is worth
less than the measurement written down. What they establish is the arithmetic:
7 words are the save ratio, 6 are ring phase, and **18 are the counter web
existing at all**.

#### p8-o001: the blocking record, and the axes covered

From the 18-word base, a force sweep of every coloured web against every colour
c1 to c14 plus the split path -- 105 forces, 71 accepted, 13 declined, 21 split,
acceptance read from the `forced` field rather than from whether the object
changed -- never reaches below 18. By L140 that is an existence proof: the
residual is below globalcolor and the lever must stop the web being formed.

Decision variable: what makes the `count--` value one IR name across the guard
and the latch. The record that blocks it is web 18's own `p1dec` line, totalsave
33 against bestcost 0, which colours unconditionally under L139 -- no source
form can raise bestcost above 33, so declining is unreachable and the web has to
not exist. Note that the *index* read of the same variable is already a scratch
temp on both sides, so the split is between the read-modify-write value and the
plain read, not between the variable's uses.

Measured and eliminated from the 31-word base, do not repeat: dropping the
`volatile` while keeping the address escape through the getter, which is what
the newly-matched `overlay1MeasureCurves` lever would predict -- it costs an
instruction (delta +4, 51 words) in every read combination, so address-taking
alone does not reproduce volatile's reloads here; `*(volatile s32 *)&count` at
the decrements, at every read, and with the declaration non-volatile (61 to 64,
delta +4 or +8); a one- and two-element `volatile s32` array with `count[0]`
(64 and 67, delta +4); `(&count)[0]` (64); a single-member struct wrapper, which
is byte-flat at 31; four guard spellings and two latch spellings; the counter
declared `u32`; and the declaration moved to the head and the tail of the list
(23 and 19). Substituting `((Overlay1OtherState *)other->state)` for the
`otherState` local at each of its four uses, in all fifteen non-empty subsets,
runs 49 to 63 -- it shifts the reference where L100 wants it but costs the
common subexpression.

#### 2026-10-01, lane a-o001: ROM-exact closure, 31 to 0

The decision variable the p8 record named -- what stops the counter's
read-modify-write value being a coloured web -- has an answer, and it is not
in how the counter is spelled.

Read off `cc -S` on mini translation units: for a plain address-taken counter
ugen emits load, copy of the old value, decrement, store, branch on the copy.
as1 then deletes the copy by renaming. Whether the load and the decrement sit
in pool registers or scratch temps depends on whether uopt promoted the
counter to a register for that block, and it does not promote in a block that
holds an indirect load or an indirect store. That is why the matched overlay 3
precedent has a scratch latch (its latch block is the loop body) and a pooled
guard. The target has scratch temps at both sites, and its latch is a join
block holding nothing but the countdown.

A load through a pointer whose value is stored to a local that is never read
leaves no instruction, yet it still rules the counter out for its block: uopt
removes the dead store after the candidacy decision. Measured on mini units:
a discarded expression statement, an empty `if` on the loaded value, a dead
store of a constant, of a pointer value, or of a global do not do it; a dead
store of an indirect load does, both after the call and at the loop tail.

The `volatile` counter was standing in for this. It reloads at every site too,
but cfe routes the old value through a temporary that globalcolor colours, one
pool web more and one scratch draw fewer than the target at each site, which
was the whole 31.

Steps, each measured on the configured TU at delta 0:

- plain counter, `while (count--)`, dead stores of `state->mode` after the
  getter and of `otherState->pending` at the end of the body: 31 to 12, with
  every scratch register and the frame exact. Which field is loaded is inert
  (seven guard fields and eight tail fields measured). A dedicated dead local
  beside a local list pointer makes the frame 0x50; storing into `pending`
  keeps 0x48.
- the remaining 12 were the `other` and `otherState` colours, the save tie
  the p8 record priced. An empty `if (other) { }` after the element load is
  one more counted reference at no instruction: 12 to 0. An or-with-zero
  probe, a self assignment and a dead tail read of `other` are not counted on
  this shape (all 12).

Not the cause, measured on this base: a 640-cell product over signature
typing, list local or parameter, declaration scope of the deltas and of
`pending`, four radius spellings and five loop forms floors at 31 without the
dead loads; aggregate and type-punned counters (one- and two-element arrays,
struct and union members, a pointer local, u32) all promote exactly as the
plain counter does.

The same mechanism should apply to `overlay1AdvanceGauge`-shaped residuals
wherever a target holds an address-taken counter in scratch temps inside a
block with no visible indirect access.

`gmake verify` printed the expected SHA1; `check-overlay-syms` and
`promotion-proof` passed (69 words, 1 of 1 relocations). Siblings in the TU
re-scored unchanged.

<!-- plateau-handoff:overlay1ConsumeNearbyPending:end -->
