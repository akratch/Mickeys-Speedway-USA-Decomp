<!-- plateau-handoff:render_epc_lock_up_display:start -->
### `render_epc_lock_up_display` plateau handoff

- source: `src/main/diCpu.c`
- score: 51 differing words
- frame: 0x50
- relocations: 127
- first mismatch: +0x18
- summary: Real use before the loop stalls: 13-iter loop 338 masked delta -296, block-local tick 51 delta 0, direct tick 53 delta 0. No leading ring draw.

Summary before this remeasure: Dead tick updates are byte-inert and compensated preincrement adds an instruction; the zero-byte leading ring draw remains open.

#### 2026-09-12, lane p9-mid: 62 -> 51, with the ring measured rather than inferred

The closure asks for "a JFG revision that actually supplies this function's C
body, or a source mechanism that delays only the level report". Neither was
needed. The residual is a ugen integer ring phase, and this is the first time
that ring has been read directly on this project rather than inferred from a
register census.

Aligner buckets before: 283 byte-exact, 57 register naming, 1 immediate-only, 4
really different, at 62 masked, size delta 0, zero displacement tax. After: 293
byte-exact, 49 register naming, 0 immediate-only, 2 really different, at 51 and
the same size. More agreement on every axis; nothing was traded.

**The frame is not the question.** `tools/frame_census.py` prints two identical
eight-slot ladders with no slot either side uses alone, so the whole residual is
allocation, as the closure said.

**The ring, measured.** The instrumented-ugen free-list trace (DKWB_UGEN_TRACE)
over this translation unit reports this function as procedure ordinal 9 with 32
integer allocations and 247 frees. Its draw order is **t6, t7, t8, t9, t0, t1,
t2, t3, t4, t5**, wrapping -- which is not the natural numeric order and is why
the register census could not read a clean cycle out of it (68 percent
coherence, three windows). Each allocation is stamped with the source line that
consumed it, so the phase is settable per statement. This is the instrument the
closure should have asked for.

**Three hoists, each removing one draw.** Putting a printf's stack-passed
argument into the already-declared `value` local before the call removes the
ring draw that argument would otherwise take: the exception-address line takes
62 to 56, the cause-table lookup 56 to 53, and the tick counter on the first
line 53 to 51. A greedy subset search over eighteen such hoists -- every
single-line cpuXYPrintf in the function that passes an argument on the stack --
converges at 51.

**One tension, stated rather than hidden.** The third hoist puts the first
line's stack argument in `value`'s own colour where the shipped code uses a ring
temp, so it disagrees at three sites it previously agreed on while agreeing at
five more. It wins on both buckets and the schedule does not move, so it is
retained, but a form that removes that draw *without* naming the value would be
strictly better and is the first thing the next lane should look for.

**The decision variable.** The shipped code's first visible ring temp is t7 --
the *second* entry in the list ugen builds, not the first. So it makes one draw
this candidate does not, before a single instruction is emitted, and the whole
remaining 49-word naming residual is that one offset propagating. A phantom pop
at the very top of the function is the shape wanted: a construct that allocates
an integer ring temp and emits nothing. The or-with-zero family does that
elsewhere in this tree; placed on `value` at four points here it is byte-inert,
so it is not that form at this site.

**Also measured and flat:** hoisting either runlink result argument, both orders
of that pair, `s32` and pointer declarations for the three out-parameters,
dropping the `value` local entirely, three spellings of the register-block
pointer, moving `value` or `regs` to the head of the declaration list (both
cost 10 words), spelling the -1 compare as a literal, and hoisting the
region-line argument (78).

2026-09-11 causal re-check: baseline remains 283 byte-exact words, 57
register-naming words, one immediate-only word, and four really-different
words, with zero displacement tax. Candidate and target have 344 words, a
0x50 frame, matching eight-slot ladders, 127 relocations each, and 125 exact
relocation identities.

Attempt 28 moved the two final report blocks as one unit because the
workbench found a four-word block shifted by 16 words. It regressed to 72
masked differences: 276 byte-exact, 54 register naming, one immediate-only,
and 17 really different. The whole-report order is not the source of the
shifted block.

Attempt 29 kept the evaluation order but saved the level result in the
already-dead `sp44` local before delaying its report. It regressed to 80
differences at 346 candidate words and added two stack-slot accesses. Delayed
reporting through the dead out-parameter local is refuted.

The explicitly authorized public JFG donor revision `efd5abb1c79636e297b831f7c2d5bf47eac39c0c`
was resolved read-only. Its changed paths are README and tooling only, with no
`diCpu.c` source, so it cannot reopen this function's body reconstruction.
The temp mapping is 68 percent coherent and its cycle is expressly not a
ring-phase precondition. The next actionable evidence is a JFG revision that
actually supplies this function's C body, or a source mechanism that delays
only the level report without adding a live value across the final call.

#### 2026-09-12 colour landscape and lattice

`tools/web_footprint.py render_epc_lock_up_display --proc 9 --every-colour`
sampled every legal colour offered by the procedure: 137 accepted probes over
16 webs, with 10 residual windows. No probe beat the unforced 51/344 baseline,
so the measured single-force colour floor is 51/344 over the complete sampled
landscape. The closest accepted probe was `p1:w213=c2` at 52; `p1:w112=c2`
scored 53 and `p1:w42=c3` scored 60. There were no winners to pass to the
lattice; the three closest accepted near-misses were measured anyway.

`tools/force_lattice.py --proc 9` measured all three singles and all applying
pairs. The pair `w213=c2 + w42=c3` scored 61, exactly additive; its blast
radius was `+1` in window 0x180 for `w213=c2` and `+8` at 0x180 plus `+1` at
0x200 for `w42=c3`, so they contend at 0x180. `w112=c2` had blast radius
`+2` at 0x100 and was additive with `w42=c3`; the `w213=c2 + w112=c2`
cell was declined because the second force was not accepted. Every measured
force worsened or preserved the baseline.

The source question is therefore not a colour choice: find a zero-byte source
construct that consumes one integer ugen ring slot before the first emitted
instruction, producing the target's leading t7 phase without naming a live
value or inserting code. The previously tested OR-with-zero family and other
natural inert forms remain flat at this site.
#### 2026-09-12, lane p24-second-mid: dead tick updates do not buy the leading draw

The base-only assignment gate passes. The configured baseline is 344 words,
zero size delta, frame 0x50, 51 raw and masked differences, first +0x18.
Its aligned buckets are 293 exact, 49 naming and two structural rows, with
no one-sided words. Both assembled streams have 127 relocation sites;
this pass does not replace the prior complete-identity audit. The previous
exhaustive colour landscape was read and not repeated.

The source hypothesis is a discarded tick-counter update after its value has
been selected as the first printf's stack argument. The counter is u32 and is
overwritten immediately after the call, so both a postincrement and a
postdecrement preserve the value passed and every later observable value,
including at unsigned wraparound. Both controls are full-TU text-identical
to the baseline and have zero aligned delta in every window: no leading draw
survives. A compensated preincrement, starting from the unsigned tick minus
one, also preserves the passed value but adds four executable bytes and
regresses to 336 masked. The aligner exposes the extra word and broad local
regressions rather than treating the positional insertion shadow as progress.

No source form is adopted. The two dead updates are eliminated too early,
and the compensated update retains an instruction instead of buying a free
draw. Stop on these three non-improving controls and the inherited probe
closure. A next attempt still needs a demonstrated zero-instruction leading
ring allocation; an update of this disposable local does not provide one.

The external-source baseline was compared against the configured whole TU
before using the private compile path, and its text is identical. Sources,
objects, scores, first mismatches and aligned maps are retained under
build/p24/render_epc_lock_up_display/. Commands: lane_status, configured
compilation, residual_map --object/--against, full-TU comparison,
finalize_plateau and tools/gates.sh. The original guarded body is retained.

<!-- plateau-handoff:render_epc_lock_up_display:end -->
