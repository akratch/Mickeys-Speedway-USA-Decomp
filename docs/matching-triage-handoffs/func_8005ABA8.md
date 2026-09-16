<!-- plateau-handoff:func_8005ABA8:start -->
### `func_8005ABA8` plateau handoff

- source: `src/main/models_5B300.c`
- score: 2/111 words
- frame: frameless
- relocations: 0
- first mismatch: +0x3C
- summary: as1 branch-delay decision remains; source-side C and phase replay are closed

#### Owned evidence and result, 2026-09-09

47 differing words to 2, at 111/111 words, frameless, zero relocations, with
every register name and every branch target now agreeing. Four source
artefacts closed 45 of them and none was an allocator question:

- `temp_f0_2` cached `instance->frameValue` for two tests that no store
  separates. The cache costs a `mov.s`; the target reads the field at both
  sites and lets uopt common the load, and the `nop` the target shows at that
  hazard slot is exactly what the copy was filling. Worth 27 words.
- The two blend stores were written sub-then-div. The target allocates the
  div's FP temporary first, so the source computes `blendStart` before
  `blendEnd`; both read only locals, so the order is free. Worth 4 words.
- The null test spelled through a cached `temp_a1` gave the loaded pointer a
  copy and exchanged `a1`/`a2` across both frame carriers and their two later
  uses. Testing `temp_v0->frame == NULL` directly lets the load keep `a1` and
  the surviving carrier take the copy into `a2`. Worth 7 words, and it is the
  opposite of the lever the previous handoff filed: the second declared
  pointer was the *cause* of the exchange once the size question was closed.
- `var_v1 = 1` written before the inner `if` of each arm rather than once
  after it moved no instruction at all, but made as1 duplicate the join's
  `move v0,v1` into two annulled delay slots the target leaves as `nop`.
  Worth 5 words, and it is the proof that this class is source-reachable.

`temp_f2_2` was an m2c-only second name; one carrier serves both `blendEnd`
reads. Removing it, and the now-dead `temp_a1`, is byte-inert.

Remaining: one as1 delay-slot decision at `+0x3C`. The target branches `beqz`
with a `nop` to a block whose first scheduled instruction is
`mul.s $f18,$f14,$f12`; as1 turns the same branch into `beqzl`, duplicates
that multiply into the annulled slot and retargets past it, which leaves the
original copy unreachable and both sides at 111 words. Twenty-eight further
shapes of the else-block head, the transition test, the declaration list,
the comparison spellings and the placement of the product are flat at 2.
Resume on the ugen text as1 consumes, not on the schedule: the `var_v1`
result above proves a statement move that changes no emitted instruction can
still flip this decision.

Tooling: the permuter's isolated scratch for this TU compiles the function at
112 words against the real object's 111, so its base score of 400 is a false
reading and nothing measured there transfers. `tools/permute.sh` on
`models_5B300.c` should not be trusted until that is fixed.

#### Second owned pass, 2026-09-09 (lane/fin-mod)

Still 2. What this pass bought is a proof of *where* the two words live and a
large set of retired levers, not a match.

- **Phase replay is exact for this TU.** `cc -S`'s ugen listing, re-assembled
  with `cfe -E` + `as0` + `as1` at the compiler-path flags (drop the driver's
  `-pic0`, keep `-O2`), reproduces the real object byte for byte. One candidate
  costs ~60 ms, so the last phase is a directly searchable space. Watch out:
  `cc -S -o <path>` ignores `-o` and drops the listing in the *current
  directory* -- move it into your scratch dir before anything else.
- **The residual is one assembler decision and nothing else.** Inserting a
  single location-counter directive (`.align n`, `.space 0`, or `.text`)
  immediately after the else-arm's label in the ugen listing suppresses the
  branch-likely conversion and yields a **byte-exact 111-word object**. Every
  other word of this function's C is therefore already the target's C.
- **The suppressor set is exactly those three directives.** Everything else
  ugen can emit inside a function body was tested at that position and at ten
  others and is inert: `.loc` (every line value, inserted, deleted and moved),
  `.livereg`, `.noalias`/`.alias`, `.mask`, `.frame`, `.file`, `.option`,
  `.verstamp`, an extra label, a padding instruction, a redundant jump. ugen
  emits `.align` and `.text` only at function *starts* (11 of each for this
  TU's 11 functions), so the suppressor is not reachable from C.
- **Physical line grouping is retired for this residual.** Grouping changes
  only `.loc` lines, and every `.loc` edit anywhere in the function is inert
  here. Do not spend another pass on it.
- **No cross-function state.** Dropping any other function from the listing
  leaves the decision unchanged, so the space is this function's own ugen text.
- **Single-line ugen-text moves do not reach it.** All 43,056 single-line moves
  (every line of the function to every other position in it) were scored. One
  variant reaches 1 -- moving the else arm's label deep into the block, which
  makes the target head a branch-class instruction and suppresses the
  duplication, leaving only the branch's own displacement wrong. That variant
  is not semantics-preserving, but it is the measurement that pins the rule
  below: **only a branch-class head, or a location-counter directive, makes the
  assembler decline.** Mapping every instruction of the function to the head of
  that block confirms it: 17 of 85 decline, and all 17 are branches.
- **Insertions the assembler removes are inert too.** A dead register-to-
  register copy vanishes from the object (the word count stays 111), so it was
  the last candidate for a phase-input difference that leaves no trace; 836
  such insertions, at every position in the function, all still convert. The
  same goes for self-copies, zero-adds, an encoded no-op and a redundant jump.
- **384 C spellings are flat.** Cross product of: the three integer tests
  plain vs `!= 0` vs `!= 0U`, the two float tests as `>=` vs negated `<`, the
  blend statement order, the disjunction spelling, `frame` carrier placement,
  `while` vs guard-plus-bottom-tested loop, the product in a local, and the
  explicit-add form. Assigning `frame` inside the else arm sinks the carrier
  copy below the test and costs 45 words: the carrier must be assigned before
  the test, as it already is.
- **How the conversion actually behaves**, measured on this function's nine
  other conditional branches: the assembler duplicates the first *scheduled*
  instruction of the branch-target block into the annulled slot and retargets
  past it. It declines only when that head is itself a branch, when the head
  has been hoisted out from under its own label by an earlier assembler
  transformation (retargeting such a branch to the next label restores the
  conversion), or when a location-counter directive stands at the head.
  Liveness of the duplicated destination on the fall-through path is *not* a
  factor (renaming it in either direction is inert), and neither is the head's
  opcode class -- integer branches duplicate float arithmetic here.
- One unexplained sensitivity worth a follow-up: replacing the byte load that
  defines the test's condition register with an ALU definition in the same
  block makes the assembler decline. Both sides of this target define that
  register with the same byte load, so it does not explain the residual, but
  it is the only input perturbation found that flips the decision without a
  directive.

Next worker: do not re-run the source-spelling or line-grouping space. The
open question is narrow -- what ugen-emittable input makes this one branch
keep its own delay slot -- and the phase-replay harness above is the tool for
it.

#### Owned pass, 2026-09-10 (lane/nm-resident2)

Still 2, and nothing in this pass reaches the assembler decision the pass
above pins it to. One small addition to the retired set: the compiler's debug
levels do not reach it either. `-g0` is byte-identical to the default and
still converts; `-g1` and `-g2` change the object entirely (176 words) and
`-g3` gives 112, so none of them is a candidate correction for this TU.

Confirmed unchanged against the current tree: 111 words on both sides, every
register and branch target exact, the residual the branch at +0x3C and its
delay slot. Do not re-run the source-spelling space; the open question
remains the one the pass above states.
#### 2026-09-10, lane o7-tight: the closure argument, sharpened to a counting one

Still 2. The phase-replay pass above proved the suppressing set is exactly the
three location-counter directives plus a branch-class head, and that ugen emits
those directives only at function starts. This pass closes the remaining half
of that statement -- the branch-class head -- with an argument rather than
another lattice, and adds the block-membership axis as a measured negative.

**A branch-class head is unreachable without adding an instruction.** as1
declines the conversion when the first *scheduled* instruction of the branch's
target block is itself a branch. A block's branch is its terminator, so the
block would have to hold nothing else; in C that means an arm whose body emits
only a conditional branch. Every construct that emits a branch and nothing else
has an empty controlled statement, and IDO folds an empty controlled statement
away before ugen -- measured again here, and independently on
`overlay20RemoveEntry` in this lane, where twenty zero-footprint reads all
vanish before the web builder. Anything with a non-empty body adds at least one
instruction, and both sides are exactly 111 words. So the head-is-a-branch route
requires a word the target does not have.

**Block membership does not reach it either.** 24 cells, compiled with this
TU's real flags (`-Wo,-loopunroll,0` included) and compared against the whole
111-instruction target text: six shapes of the else arm's head -- unchanged, an
`if (1) { }` region, a `do { } while (0)` region, a bare brace pair, the
compound assignment written out long, and the product written as the left
summand -- crossed with four polarities of the transition test, including the
two that exchange which arm is the fall-through and which is the branch target.
The ten cells that keep the recorded polarity are flat at 2; `w5` (product as
the left summand) is 4; every inverted-polarity cell is 95, because exchanging
the arms rewrites the whole function. So neither opening a uopt region at the
head of the target block nor moving the branch to the other arm changes the
assembler's decision.

Read together with the phase replay, this function is now closed under C as
firmly as this project can close anything short of an as1 trace: the decision
needs an input ugen does not emit, and the two source-side routes to that input
(a directive, a branch-class head) are each excluded by measurement. Anything
further belongs in an as1 instrumentation pass, not in a source lattice. Do not
spend another lane on spellings.

#### Owned pass, 2026-09-11 (lane/w3-low): the assembler-flag question closed, one recorded sensitivity withdrawn, and the suppressor mechanism named

Still 2, nothing adopted, the source file is unchanged. Before and after are the same
measurement: 444 bytes, 111 of 111 words, size delta 0, positional masked 2, aligner
buckets 109 byte-exact, 0 register naming, 0 immediate only, 2 really different, first
structural difference +0x3C.

**The branch-likely flag hypothesis is refuted, and at function scope rather than file
scope.** Census of this translation unit's own text in the ROM, all eleven functions,
full coverage of its 0x1010-byte text block: 96 conditional-branch-class words, of
which 29 are in a branch-likely form. Ten of those 29 are inside this function itself,
out of its thirteen conditional branches. A per-file flag that disables the assembler's
branch-likely conversion is therefore refuted by trap 10 whatever it scores, and so is a
per-function one. Per function the counts are: 0, 0, 0, 1, 2, 2, 0, 10, 7, 7, 0.

The flag exists, and was measured anyway so that the refutation is concrete. as1's
option table, recovered from the shipped binary, carries -nobopt, -no_branch_target,
-noxbb, -aggr_xbb, -nopeep, -peepdbg, -noswpipe, -swpdbg, -multi_issue, -noglobal,
-nosymregs, -newhilo, -domtag, -fpstall_nop and roughly ninety more, and the driver
passes them through with -Wb. Two of them, -nobopt and -no_branch_target, have exactly
the effect the hypothesis wants: they reproduce the target's own two words at +0x3C and
+0x40 exactly. They also decline the ten conversions the target *has*, so the function
goes from 2 differing words to 20 in ten branch-and-delay-slot pairs, and five other
functions in the TU stop matching. -noxbb costs 21 words and six functions, -aggr_xbb
and -multi_issue change the function's size, and the other nine flags tried are either
fully inert, seven of them, or leave this function at 2 while unmatching others, which
is what a peephole switch and an at-compression switch do. No assembler option
reaches this residual. The question is closed; do not reopen it.

**The one recorded input sensitivity is an artefact, and should be struck from the
retired-lever list as evidence.** The previous pass recorded that replacing the byte
load which defines the test's condition register with an ALU definition makes the
assembler decline, and filed it as the only unexplained perturbation that flips the
decision without a directive. It does not flip the decision. Every such variant changes
the function's *size*: the ALU forms come out at 109 or 110 words against 111, because
the assembler's own peephole recognises the copy, propagates it into the branch, and
deletes the defining instruction, after which the branch's operand is long since live,
the block's spare instruction fills the delay slot locally, and there is no empty slot
left for the duplication to fill. The earlier reading took a fixed byte offset in a
shorter function, which is trap 2. Re-measured with a structural locator instead of an
offset, and scored across the load and ALU families: the three other byte and halfword
and word loads all still convert at 111 words, and every ALU form shrinks. So the rule
is not load-versus-ALU and not a latency class; it is that in those variants the delay
slot stopped being empty. That axis is spent, and it never was an axis.

**What the scheduler trace says, and what it does not.** The assembler's own node dump
is available through the driver as -Wb,-R, and it prints, per basic block, every node
with its instruction word, its source line, its predecessor count, its critical-path
length, its hazard, and its after-node list with per-edge latencies, followed by the
whole list-scheduling decision sequence. For the contested branch the block holds three
nodes: the frame carrier copy with no edges at all, the byte load with a single
after-edge to the branch at latency three, and the branch itself. For the target block
it holds nine nodes; the duplicated multiply is a source with one after-edge at latency
seven and the largest critical-path length in the block, which is why the scheduler
picks it first and therefore why it is the instruction that gets duplicated. There is
no tie to break: its critical-path length is the unique maximum among the block's three
entry nodes. The block's only memory pairs share one base and one displacement, so the
one L125 ordering edge present is a same-base store-to-load edge that the disambiguator
resolves correctly, and there is no different-base pair in either block for L125 to
order conservatively.

The decisive negative is this: the node tables and the entire scheduling sequence are
**bit-identical** between a run that converts and a run that declines. Diffing the two
traces over the whole function yields exactly one difference, and it is not in any node
or any edge. So the duplication decision is not downstream of a dependence edge and not
downstream of a scheduling tie-break. Which instruction is duplicated is DAG-determined;
whether duplication happens at all is decided somewhere the node dump does not print.
The other debug options tried, -xbbdbg, -peepdbg, -swpdbg and -diag, print nothing for
this TU, so -R is the only trace available.

**The suppressor mechanism is now named, and it explains all of them.** The one
difference between the converting and declining traces is an extra basic block with no
nodes in it, standing at the head of the else arm. Every known suppressor produces it:
a location-counter directive splits a zero-node block off at the label, and the
conversion then has no first instruction to duplicate. Re-measured at the block head,
the suppressor set is four items, not three: the three location-counter directives, and
an empty noreorder-then-reorder pair, which is byte-exact at 444 bytes and zero
differing words like the others and produces the same empty block in the trace. A
duplicate label, a live-register annotation, a debug-line directive, a mask, a frame, an
option, a scope-begin and a volatile set are all inert; a file directive is a wrecker
rather than a suppressor at 43 differing words, and an unclosed noreorder truncates the
function to 93 words. The fourth
suppressor is no more reachable from C than the other three: this compiler has no inline
assembly statement at all, and the two spellings tried compile as ordinary calls, adding
a frame and 20 words. The second decline rule, a branch-class head, remains separately
argued in the pass above.

So the closure survives, with its reasoning sharpened from "three directives ugen emits
only at function starts" to "the branch's target block must hold zero nodes when the
branch optimiser runs, and nothing a C source can say produces a zero-node block".

**What is left, and what to spend it on.** One in-function counter-example is still
unexplained and is the only remaining lead: the conditional branch at +0x180 declines
the conversion on *both* sides, with an empty delay slot and a target block whose head
is an ordinary register copy, and that same block's head is duplicated successfully by
two other branches in the same function. Whatever second condition makes that one branch
decline is the same kind of condition the contested branch needs. It is not distance, it
is not the head's opcode class, it is not a first-use cap, and it is not visible in the
node dump. Finding it means reading the assembler's branch optimiser itself rather than
its output. Do not spend another lane on source spellings, on the phase-input move space,
on debug levels, on block membership, or on assembler flags: all five are now measured
out.

#### 2026-09-11, lane p6-tight: confirmed unchanged, no new search

Re-measured against the current tree: 444 bytes, 111 of 111 words, size delta
0, positional masked 2, aligner buckets 109 byte-exact, 0 register naming, 0
immediate only, 2 really different, first structural difference +0x3C. The two
words are the recorded pair: this candidate's conditional branch at +0x3C takes
the annulled form with the target block's first scheduled instruction copied
into its slot and its displacement moved past that copy, while the ROM branches
plainly with an empty slot; the duplicated instruction itself stands
identically in both objects further down, so the two sides differ in exactly
those two words and in nothing else.

Nothing was searched this pass. The 2026-09-10 and 2026-09-11 closures argue
the space shut on two independent grounds -- the assembler declines only when
the branch's target block holds zero nodes when the branch optimiser runs, and
no C source produces a zero-node block; and the per-file and per-function
branch-likely flags are refuted by this translation unit's own census. The
remaining lead is the in-function counter-example those passes name, and it
needs the assembler's branch optimiser read rather than another lattice.


#### 2026-09-12, lane p19-reopen: two-word assembler closure survives

The assignment gate returned base-only. Fresh alignment reproduces 111 owned
words, delta zero, buckets 109 exact / 0 naming / 0 immediate / 2 really
different, zero displacement tax, frameless, no relocations, first +0x3C.
There is no register-substitution component for L142, L139 or L131 to move.
L144 does not supply the assembler block-boundary suppressor established by
the preceding controlled experiments. No new source spelling was attempted.

This is a scope decision under ADR 0018, not a claim that two words are
unmatchable. The inherited assembler-input existence argument survives the
new allocator/reload laws, and its accepted diagnostic output was not counted
as untouched compiler output. The remaining concrete lead is still the
unexplained in-function branch at +0x180: inspect the assembler optimizer's
second decline condition before proposing another C form. Do not repeat the
recorded directive, source-order or flag sweeps.

Validation commands for this pass: `tools/align_symbol.py`,
`tools/register_census.py`, `tools/score_symbol.py`,
`tools/finalize_plateau.py`, and `tools/gates.sh`. The guarded C and assembly
fallback remain; the full-ROM verification covers that fallback and earns
zero new matching bytes. Source and handoff are the only files refreshed.

#### 2026-09-12, lane `p23-lastmile4`: current residual classification

The current candidate remains 444 bytes, 111 words, delta 0, frameless, with
zero relocations and two masked words. Both aligned rows are structural; there
are no naming or immediate rows. `web_footprint.py --every-colour` correctly
refuses this procedure because it has no allocator decisions, so the exhaustive
colour winners list is empty and allocation is not an applicable axis.

The named remaining source question is still the retained as1 branch-decline
decision: what source-reachable ugen text makes the assembler leave the target
delay slot unfilled without changing the 111-word geometry.

#### 2026-09-13, lane `j1`: authenticated draw census and carrier deletion

Baseline recompiled at 111 words, delta zero, masked 2, with alignment
109 exact, zero naming, zero immediate and two structural rows, first +0x3C.
The traced full-TU text is byte-identical to the configured stock text.
`allocator_trace_receipt.py --map-only` maps the retained named Ucode and
index to procedure 7, not the census default of 0. The prior claim that this
function has no allocator decisions is withdrawn: procedure 7 has twelve
decisions; procedure 0 is a different function. No colour sweep was repeated.

The corrected `draw_census.py --proc 7` baseline has 24 draws and 164
emissions. One source experiment deletes the declared frame pointer and
inlines its two loop-flag accesses, retaining physical line positions. This
tests whether eliminating the carrier before the contested branch supplies
a different emission schedule without extra geometry. Draws rise to 27:
the frame-null test gains one, and each loop-flag access gains one; the
transition test itself is unchanged. Emissions remain 164 overall, with
the carrier assignment's two emissions replaced at the flag reads.

The candidate grows by four bytes and scores 81 masked words. The aligned
comparison against baseline adds twelve naming, six immediate and one
structural row, with two new candidate-only positions and one target-only
position. This is a measured regression, not progress hidden by positional
shift. The source and object are retained in private lane scratch and the
original guarded candidate is restored. Attempt count: one.

Stop under ADR 0018: the available carrier-deletion hypothesis is eliminated;
the extensive prior branch-optimizer closure still blocks the other source
axes. This does not prove all possible schedules closed. Next action remains
the as1 decline-condition investigation at the in-function counter-example
identified above. Commands: assignment gate, aligner, score, retained Ucode
procedure receipt, draw census before/after, aligned residual comparison,
`finalize_plateau.py`, and `tools/gates.sh`. ROM verification covers the
assembly fallback; this pass earns zero new matching bytes.
#### 2026-09-16, lane lm-c: read only

Baseline reproduced at 2, delta zero, first +0x3C. No cycle spent; the as1
zero-node-block closure stands and nothing in this lane's readings reopens
it.
<!-- plateau-handoff:func_8005ABA8:end -->
