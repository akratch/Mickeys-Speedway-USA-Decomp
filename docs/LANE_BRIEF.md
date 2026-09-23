# Standing lane brief

Read this once at the start of a lane. Your dispatch message carries only your
targets, their measured numbers, and whatever is specific to them; everything
that is true for every lane lives here so it is written once and stays correct.

This file is the authority. If a dispatch message contradicts it, the dispatch
message is stale — say so in your report.

## What you are doing

Mickey's Speedway USA, a byte-identical matching decompilation for the N64. The
ROM must rebuild to SHA1 `507341c0a40ca3e9a7cee969b396ee53facfb548`. Read
`CLAUDE.md` in your worktree before anything else; it carries the build rules,
the clean-room policy, and the overlay promotion sequence.

Work only in your own lane worktree. Never touch the integration worktree or
another lane. Do not merge, rebase, or push — the coordinator integrates.

## Measure the cause before choosing a lever

`tools/align_symbol.py <symbol>` aligns your candidate against the target on a
register-erased shape and splits the residual four ways. Run it first, and
again after every adoption:

| bucket | what it means | where the lever is |
|---|---|---|
| byte-exact | agreed | — |
| register naming | same instruction, different register | allocation |
| immediate only | same instruction and registers, different displacement or constant | the frame, or a wrong literal |
| really different | different instruction, or present on one side only | structure |

It also prints a **displacement tax** — how much of the positional count is
pure shift — and the offsets where instructions are surplus or missing. On a
size mismatch that last pair is the whole question.

`tools/register_census.py <symbol>` says *which* registers the two sides disagree on and whether they form a cycle. A closed cycle over the temp ring is one ring-phase fact with a single cause (L127, fixable from source at zero byte cost); scattered substitutions are per-web colour questions. Reading a cycle as N colour problems is how a lane spends a day on the wrong axis -- on one function 195 of 279 words were a single four-cycle.

  **Read it in address order.** The tool reports how much of the residual one global mapping explains and how many windows are needed. One window function-wide is a single ring-phase fact; a mapping that changes every few sites is per-iteration consumption and each window is its own question. Measured: a function whose dominant mapping printed a clean four-cycle needed **six windows at 60%**, while a sibling two functions away was **one clean shift at 83%**.

  **Read the coherence line, not just the cycle.** A cycle in an incoherent mapping is not a ring phase: a transfer was applied on cycle presence alone and refused. The lead that transferred reads 88% of substitutions following their source's dominant target; the sibling that refused reads 51%, with one source register splitting three ways. Check the size delta too -- the lead was +4 with identical frames, the sibling -76 with frames 0x10 apart.

`tools/residual_map.py <symbol>` splits the same residual by **address**: differing rows per window, naming/immediate/structural, the candidate-only and target-only offsets that bracket every +1 run, and a register substitution census **per window** with its cycles. Use it whenever a whole-function census reads as incoherent, because a rotated tail and an un-rotated head average each other away: on the overlay 58 whale the function-wide reading was "flat per-web colour, twenty-odd windows", and split at the single candidate-only word it was 150 of 212 slots in one closed nine-cycle after that word and no cycle at all before it. One extra instruction had rotated the free list for 1,196 bytes. `--lo`/`--hi` restrict every section, which is how you price a region.

`tools/residual_map.py` also takes `--object <path>` to map a *retained* object
instead of recompiling the tree, and `--against <path>` to print the **aligned**
per-window delta between two objects. That delta is the reading to trust when
comparing two candidates: `force_lattice`'s blast radius is positional, which
is right for locating a force (both objects face the same target, so an
insertion's shadow cancels) and wrong for ranking a window (L155).

`tools/web_footprint.py <symbol> --trace <base allocator.log> --out DIR` probes
**every** coloured web of a procedure once -- forcing it to the cheapest legal
colour of its own kind -- and inverts the result into a `window -> webs`
nomination table. That table is what turns "this window has eleven naming rows"
into a list of forces to try. Read L157 before choosing a probe colour by hand
and L158 before believing any lattice's floor. One compile per web; the overlay
58 procedure colours 139 of its 431 decisions, so a full map is 139 compiles
and is a lane's job, not a sweep you run inside another sweep.

`tools/draw_census.py <symbol>` wraps the freelist trace below into a per-SOURCE-LINE
census: draws, frees and emissions for every line of the function, plus the draw
order as a sequence. `--save a.json`, change one thing, `--save b.json`,
`--compare a.json b.json` names the lines whose draw or emission count moved.
**A line whose draws did not move was not moved by your edit**, however much the
score moved -- and a score moves for causes the edit did not create. This is the
instrument for the blockers the colour axis leaves behind: "pre-colour web
creation order", "the emission-order blocker", "ring release order". It reads
what OUR source makes the compiler do; the target side still comes from
`residual_map.py`.

`docs/WHALE.md` is the standing plan for `func_overlay_058_F000138C_18B0574`,
the tree's largest unmatched function. Read it before any pass on that symbol:
it carries what the 187 residual is made of, which axes are closed with
evidence, and the one that is not. **The allocator makes 395 recorded decisions
on it and 254 of them are `decision=split`, which no `CDX_FORCE` addresses** --
every landscape ever run on this function measured only the 141 colour
decisions.

`tools/frame_census.py <symbol>` censuses both sides' stack slots and diffs
them: each side's ladder from the frame top down, the slots only one side uses,
and shared slots with different traffic. Run it on any residual with a frame or
displacement component -- three of six closures re-tested on 2026-09-11 were
home-set problems nobody had measured. It reads the objects, so it is symmetric;
`cc -g3` names which of *your* slots are declared locals rather than compiler
temps, which is the other half of L118.

`tools/score_symbol.py <symbol>` gives the positional masked count, agreeing
with the ranking by construction. **Work the masked number**, never the raw
one: the difference is relocation artefacts, already partitioned and stored.

**Concurrency.** `score_symbol.py`, `align_symbol.py` and `frame_census.py`
each swap `nm_ranking`'s work directory for private scratch, so any number of
them can run at once in one worktree — measured: three in parallel return the
same numbers as a serial run. **`nm_ranking.py` run directly does not**, because
a whole-queue pass is meant to own the tree. So do not start a `--refresh-stale`
or `--out` pass while a harness is measuring in the same worktree; that is the
one combination that collides.

  **The isolation is the work directory, not the source file.** A *sweep* --
  `blockclimb.py`, or any candidate harness -- writes the TU's own `.c` in the
  tree between compiles, so anything else that measures that TU while it runs
  reads whichever variant happened to be on disk. This is not the scratch
  collision the memory note records; the work directories are private and the
  numbers still come back, plausibly, and wrong. It cost this lane an identity
  gate that read FAIL with 4 words differing and then PASS byte-identical on the
  same source once the sweep had finished. **One sweep per TU at a time, and no
  measurement of that TU beside it.** Different TUs in parallel are fine, and a
  sweep on one TU beside a measurement of another is fine.

## Go to the records early, not after the lattice fails

**This is the biggest measured difference in how lanes spend their time.** The
instrumented-`uopt` loop — dump the decision records, force the decision you
think is wrong, score the *forced object directly* — has been **decisive in
about twenty minutes per function**, twice in one lane, and the harness runs
roughly 1.5 candidates/sec against the full configured TU. Over the same period
spelling lattices have overwhelmingly come back flat: a 64-cell
commutativity lattice, a 128-point region sweep, a 162-placement sweep, and one
closure recording 4,190 flat forms whose function then moved on a single
hoisted definition.

So on a naming residual, **the order is records first, lattice second**:

1. Confirm the instrumented toolchain's `.text` is byte-identical to the tree's
   object. That is the identity gate; an ungated instrument attributes
   decisions to the wrong pass, which is worse than having no instrument.
2. Dump the records and read the ladder — each web's `save`, `nocs` and
   `totalsave`, in the order they are offered a colour.
3. Force the decision you believe is wrong and **score the forced object
   against its target directly**. `score_symbol.py`, `align_symbol.py`,
   `frame_census.py` and `register_census.py` all recompile the TU with the
   configured command first, so they report the *unforced* build — one lane
   read the same score for twelve consecutive forces before noticing.
4. **Set `CDX_PROC` alongside `CDX_FORCE`.** Without it the force is *silently
   ignored*: the record reads `forced=-2` and the object comes back
   byte-identical — which is indistinguishable from L101's already-forbidden
   decline, so a dropped experiment reads as a real negative. Verify acceptance
   by reading the `forced` value (`-1` accepted, `-2` never applied), never by
   whether the object changed. A sweep inherited from a handoff is only evidence
   if it recorded acceptance.

What this buys is a *number*: "this one decision is worth N words". Two
functions were priced at 48 → 9 and 26 → 11 this way, both at size delta 0.
That converts an open-ended search into arithmetic — you then know what the
source form has to achieve, and often that the requirement is unreachable,
which is equally valuable and far cheaper than discovering it by sweeping.

A worked example of the second outcome: on one function the force proved 15 of
26 words were a single ranking, and the ladder then showed the wanted web
carries one occurrence more than its rival at the same net. Since `nocs` is
non-decreasing, no added basic block can reverse the ranking — a closure with a
named decision variable and an argument, reached in one sitting, where a
spelling lattice would have returned "flat" with nothing learned.

## The call test — ask this before any allocator work

Every procedure that issues a call emits **p1 allocator records only**; most
leaves emit **p2 only**. 59 procedures classified with zero counterexamples
until 2026-09-23, when `func_80049B14`, a leaf, emitted 14 p1 decisions and
changed regime (14 coloured webs to 4) when its size gap closed. So for a leaf,
read the record kinds once before trusting the p2 rule; a leaf whose webs are
global candidates is on the p1 axis.

- **Your function contains a call** ⇒ p1 only ⇒ the axis is the `save` ratio
  (L100) **only for colours the web is actually offered**. What this retires is
  **L106 specifically** — ugen's p2 web-number ordering — and nothing else.
  A closure that swept L106's axis on a p1-only function measured something the
  function does not have.
  **It does NOT retire statement order.** This brief previously said
  "definition position, declaration order and statement order decide nothing",
  and that is false as written: statement order reaches **as1's scheduling** and
  **ugen's emission order**, neither of which the call test touches. On one lane
  statement order was decisive on **all seven** functions it closed, and a shard
  carrying the old phrasing had told the next lane not to look. Read the
  retirement narrowly.
  **Check the offer list before the ratio (L142).** A web live across a call is
  denied `v0`/`v1` *and exactly those argument registers the spanned calls
  load*. The denial is recorded in the web's **`forbidden` mask**; it also shows
  as absence from the `p1cost` list, because that list omits every forbidden
  colour. Calls loading `a0`/`a1` give a list starting at c5; calls loading
  `a0`–`a3` give c7.
  **Two limits, both measured:** the denial is **per call, not per call-set** —
  one spanned call can carry it while another does not, and one argument on one
  call is worth exactly one colour. And it reaches **only the caller-saved head
  of the colour table**, so where the contested colours are `s`-registers the
  arity lever is entirely inert (1, 3 and 4 arguments left every web's `save`,
  `nocs`, `totalsave`, mask and colour bit-for-bit identical). **Establish which
  bank the contest is in before spending a pass on arity.** If the register you want is not in the list,
  no ratio, carrier or spelling reaches it and raising `totalsave` will move the
  number without ever producing the colour — two closures were written demanding
  exactly that. The lever that does exist is **the arity of the calls inside the
  web's range**: dropping one argument moved a mask from `0x7803e000` to
  `0x7003e000` and globalcolor then chose the wanted register at an unchanged
  save. The float bank behaves identically — `f12`/`f14` are argument registers,
  so a call taking two floats masks them both.
- **Your function is a leaf** ⇒ p2 only ⇒ the axis is ascending web number, and
  moving a defining statement is a lever (L106).

This costs one look at the body and it retires or opens a whole family of
levers. Two lanes were sent after L106 on p1-only functions before this was
known; both spent a sweep finding out.

## Treat a closure as a claim, not a fact

A plateau handoff saying "N forms measured, all flat" is evidence about the
axis its author varied. The measured base rate here is poor: of sixteen
closures re-tested, **ten fell**. One recorded 4,190 flat forms — an exhaustive
4,096-cell lattice plus 82 declaration-order forms — and the function then
matched on a single hoisted definition. Another was recorded as a 562-word
structural residual and matched on a compiler flag.

So read each closure adversarially and ask:

1. **Does it name a decision variable?** If it says "no source form reaches it"
   without naming one, it is not a closure. The closures that survive re-testing
   name one; the ones that fall do not.
2. **What did it vary, and therefore hold fixed?** Declaration order and
   definition position are different axes. A spelling lattice holds block
   membership constant by construction. A force sweep says nothing about
   statement placement.
3. **Was it written before the law that reaches it?** `tools/reopen_candidates.py`
   lists closures that predate a law touching their territory.

Read the in-source `/* PLATEAU-HANDOFF:<symbol>:start … */` block and
`docs/matching-triage-handoffs/<symbol>.md` for every target before touching it.
Re-running a lattice another lane already exhausted is this project's single
largest measured waste.

## The laws

Full statements and receipts are in `docs/compiler-laws/ido-5.3.md` in the
workbench at `~/Desktop/dev/n64-decomp-workbench`. Skim the index; do not read
it end to end. The ones that carry most of the weight:

- **L100** — a web's save is `totalsave/nocs`, and a symbol boundary moves both
  terms at zero width. Merging a per-arm declaration to function scope raises a
  save; naming what was one shared cfe temp splits it; a reference inside a
  loop weighs ×10. Ties keep the incumbent, scanning in ascending web number.
  One function went 21 → 5 on this where **each edit alone was a 31-word
  regression** — do not discard a direction because one half regresses.
- **L101** — a web whose span reaches a call result is not *offered* v0; it is
  absent from the `p1cost` list rather than forbidden in it. A declined force
  has three kinds: nothing in the record ⇒ split the web; a `forbidden` mask ⇒
  genuine interference no ratio or spelling reaches; already forbidden at
  decision time ⇒ the force never applied and **the object returns
  byte-identical, so the experiment proved nothing**. Check a force was
  *accepted* before reading an unchanged object.
- **L114, corrected 2026-09-11** — globalcolor's colour table reaches further
  down the caller-saved registers than this brief used to say, and **the
  boundary is per procedure**. One procedure decodes as c1 `v0` … c9 `t2`, c10
  `t3`, c11 `t4`, c12 `t5`, c13 unnamed, c14 `s0`, c15 `s1`, with two webs
  carrying `color=10 reg=t3` outright — so `t3`–`t6` are priced candidates a
  force can reach there, and only `t7`–`t9` and the float ring sit outside.
  **Decode the table from the `p1color`/`p1cost` records on your own procedure**
  rather than assuming a boundary. Splitting a naming residual by register bank
  is still the right first move; just do not treat a `t`-register row as
  unreachable until the records say so. Roughly a sixth of a naming residual is
  colour, and that figure is a **floor** — a force sweep never offers a colour
  another web has forbidden, so free the interferer before concluding a lever is
  out of reach.
- **L115 + L131** — a live range is formed per *IR name*, and interference is a
  block-set intersection. Reusing a local that is already live elsewhere imports
  its interference at zero width and no instruction; adding a fresh one does
  not. Which existing local you pick is the whole decision.
  **A repeated expression is one name** (L131, 2026-09-11): the same address
  expression written at three sites is *one* range whose hull spans everything
  between them, and whose uses are every site's uses, not the ones beside any
  single occurrence. So deleting the local that holds it does not split it, and
  neither does giving each site its own local — to split the range the
  *spellings* must differ. Measured on the o101 quadruplet: removing the pointer
  local moved nothing; changing the spelling at the store sites moved 8 words.
- **L112** — an unobservable array length is a free parameter the frame identity
  solves for: `frame = round8(fixed + block + temps)`, `block = base +
  element_size × count`. Put the target's frame in, read the count out. Check
  every dimension the ROM cannot see before concluding the frame has no cell to
  give.
- **L99** — stack homes descend from the frame top in declaration order, and
  only a memory-class local gets one. The offset is a linear readout, so sweep
  and solve rather than guess. Frame *size* is a count, not an order.
  **Corrected 2026-09-23:** an unused `s32` is *not* reliably eliminated
  before the frame is sized. Five Track B matches (`func_8000DB34`,
  `func_800133FC`, `wakeDraw`, `func_800479D4`, the overlay 20 match) depend
  on unreferenced `s32` pads to land the target's homes; removing three from
  `wakeDraw` moved its frame from 0x88 to 0x78. Treat unreferenced locals as
  a frame lever, sweep their count and position with `frame_census.py`, and
  do not assume the compiler drops them. `align8(4N)` hides a one-slot
  change.
- **L150** — **a deleted no-op is not side-effect free.** as1 removes it *by
  renaming its producer's destination*, so a no-op placed to buy a ring draw
  also moves whatever produced its operand off the colour it held. That is why
  this family keeps failing where it looks like it should work: the edit buys
  the draw and breaks an agreeing register at once. Precondition: the operand's
  producer must be a value whose destination you are willing to lose. Two facts
  come with it — **ugen draws a ring register immediately before each
  instruction it emits, so `cc -S`'s listing order IS the draw order** (read it
  off the listing, do not infer phase from a census); and **L147 only opens a
  function whose references are split across blocks** — where they already sit
  in one block `nocs` is at its floor of 1 and a block reading can only divide
  the save further.
- **L149** — **count draws, not registers.** A ugen draw and the instruction
  consuming it are separable: as1 can fold the operation into a neighbour and
  delete it while the draw stays spent, so every later row is one position off
  and nothing in the emitted code shows why. No register census or spelling
  lattice can see it. Use `DKWB_UGEN_SCHED=1`'s `DKWB-FREELIST …
  ALLOC_GP_RESULT` records, stamped per source line, and count draws per
  iteration **before** classifying a per-row phase. The generator to recognise
  on sight is a redundant mask on a narrow type: reading a `u8` local already
  emits the `and`, so `(f32)(u32)(x & 0xFF)` masks twice and as1 folds the pair
  into one `andi`. Removing the second mask emptied a naming bucket 87 → 0 and
  closed three 1,520-byte siblings.
- **L145** — **to put a value in a ring temp, delete the carrier.** A declared
  local is a symbol and a symbol is *never* handed a ring temp (L130), so no
  probe, no-op or qualifier placed through a local supplies a ring draw. Writing
  the uses as the expression itself — the global's own subscript, with no
  pointer or index local at all — makes the expression on both sides of a call
  **one IR name** (L131), so it is one range and both occurrences take the one
  caller-saved register, while the pre-call read lands in a ring temporary.
  Split and merge are **not symmetric**: a spelling lattice explores only the
  split side, and on the split shape a force onto the wanted colour is *declined
  with a forbidden mask* because the two address webs genuinely interfere. Five
  functions were promoted on this, priced at delta 0: index locals everywhere
  179, mixed 165, **no locals at all 98**.
- **L155 (2026-09-12)** — **a positional score counts the shadow of an
  insertion, and no colour can move it.** One extra or missing word makes every
  following word mismatch *by position* while aligning perfectly, so a
  positional count charges the whole span to whatever window it falls in. On
  the overlay 58 function 81 of 227 positional words sat in two windows
  bracketed by one-word insertions; their aligned residual is 22 rows. A force
  lattice that reads its floor off the positional score therefore reports a
  floor that is partly unreachable *by any colour at all*: 185 there, of which
  81 was insertion shadow that five forces could not and did not touch. **Rank
  a window by `tools/residual_map.py`'s aligned rows, never by the positional
  count**, and read the candidate-only/target-only offsets it lists first --
  they are the source-shape questions, and they are usually a different lane's
  work from the colour questions.
- **L156 (2026-09-12)** — **disjoint blast radii predict additivity, so n
  measurements replace 2ⁿ.** A force's blast radius is the signed per-window
  change in residual between its object and the unforced one
  (`tools/force_lattice.py`). Where two forces' radii share no window, their
  pair is additive. Measured on all ten pairs of the overlay 58 lattice: at a
  window of `0x80` every pair reads disjoint and every pair measured an
  interaction of exactly zero, and at word granularity no two of the five
  forces move the same word. **The width is the whole reading**: at `0x200`
  the two forces nearest the entry shared a window and read as contending
  though their scores did not, so a collision at a coarse width is a question
  for a narrower one, not a verdict. Use this to stop paying for the
  higher-order cells, and to predict a pair before compiling it.
  **Confirmed in both directions on a second lattice (2026-09-12).** Seven
  forces on the same function, 21 pairs, 11 of them measured: nine read
  disjoint and measured an interaction of exactly zero, and the two that read
  as contending were the only two antagonistic pairs -- `w75+w950` collided in
  three windows and measured `+43`, `w379+w963` collided in four and measured
  `+10`. 11 of 11 predicted from 7 single measurements. The original five were
  all additive, so "disjoint implies additive" had never been tested against a
  positive case; it now is. The antagonistic pairs also read as near-duplicate
  radii -- `w963` moves `0x1700-8 0x1780-2 0x1900-1`, exactly `w379`'s window
  set -- which is the signature of two webs competing for one slot, so **a
  radius that is a subset of another force's is a rival, not an addition.**
- **L157 (2026-09-12)** — **the colours a web can be forced to are its own
  `p1cost` table, not its `available0/1` mask.** The mask is the state at the
  moment that web was decided and a force overrides the decision, so it
  under-reports: on overlay 58 it called three of the five colours a lane
  *successfully forced* illegal, while all 139 coloured webs list their own
  final colour in their cost table. The `kind=` field on the same rows
  separates caller-save from callee-save; probing a web across that boundary
  rewrites the prologue, moves the function's size and shifts the insertion
  shadow, which throws the cell away. **Screen a nominated force against the
  web's cost table, and prefer its own kind.**
- **L158 (2026-09-12)** — **nominating five webs out of 139 is not a floor, it
  is a sample.** The overlay 58 procedure takes 431 p1 decisions and records
  139 `p1color` rows; a lane hand-picked five from the cost table, swept all 31
  subsets and reported "diagnostic floor 185". The five moved 10 of 22 windows
  and left the two largest naming blocks (+0x800 with 11 aligned naming rows,
  +0x1200 with 12) untouched, and one of them — the count/stride force — *added*
  three naming rows at +0x1200 while saving ten overall, which score-level
  additivity hides completely. `tools/web_footprint.py` probes every coloured
  web once and inverts the result into a window→webs nomination table, which is
  the step that was missing. **Before believing a lattice's floor, ask what
  fraction of the coloured webs it sampled and which windows it never moved.**
- **L159 (2026-09-12)** — **the best set of forces is a PACKING over blast
  radii, not the top of the winners list, and a better single force can be a
  worse member of a set.** Measured on the complete 1,875-probe landscape of
  the overlay 58 procedure: `w225=c20` scores 217 alone and `w225=c14` scores
  220, yet the five-force set holding c14 measures **185** and the one holding
  c20 measures **192**. `w225=c20` shares its radius *byte for byte* with
  `w379=c20`, so taking it duplicates a slot already covered and abandons the
  separate region at `+0x780` that only c14 reaches. Greedy by single score
  gets this wrong every time. Two constraints make the packing correct: radii
  must not overlap, and **at most one colour per web** -- without the second
  the packing cheerfully proposes `w225=c14` and `w225=c20` together and
  predicts a score no compile can produce. `tools/web_footprint.py --report`
  computes it; on this function it returns exactly the five forces a lane found
  by hand, and the 185 it predicts is the measured value. **A set of forces
  whose radii are identical is one question with several handles** -- three
  webs share the `+0x1700` radius here -- so nominating two of them pays for
  both and gets neither.
- **L160 (2026-09-12)** — **a declared carrier can HIDE the web the target
  needs; delete the declaration and let IDO generate the value.** When the
  decision records say a web took the lowest free colour and the ROM took a
  different one, the usual cause is that our source DECLARES a cursor or an
  intermediate that the target's source does not. Writing the access as an
  indexed subscript instead of a walking pointer makes strength reduction
  create the cursor, and the array-base web then survives to take the colour
  the target wants. A named intermediate also changes which of two tied webs
  comes first in the priority order, which is what decides the tie: on one
  function the old declared scale had total save 11 and beat the input angle's
  10, and removing it left the angle first among the tied FP webs so both took
  their wanted colours with no force at all. This is L145 generalised off ring
  temps -- **try it first on any "this web took the lowest free colour"
  residual.** It matched `overlay1FindType47ByAngle` and
  `overlay89InitializeEffect` in one edit each, both of which had first been
  reduced to a proved zero-scoring force by the exhaustive landscape. The route
  is: exhaust the colour axis, get a force that scores 0 at delta 0, then ask
  which declaration is standing in the way of the compiler choosing it alone.
- **L154 (2026-09-12)** — **a web's number follows its TYPE first and its first
  USE second, and the source decides which type a value gets.** Measured on one
  leaf with the decision records: its address constants are type-1 webs numbered
  above every type-3 symbol web, and within type 1 they are numbered in order of
  first occurrence in the INSTRUCTION STREAM, not by constant-table slot. A loop
  bound written as a local initialised from an address constant is therefore
  propagated away, re-created at its use in the loop tail, numbered last,
  coloured last and hoisted last — three symptoms, one cause, and no
  declaration, statement or grouping order touches any of them. The same bound
  produced by linear-function-test replacement, from a source that declares only
  an INDEX and lets uopt strength-reduce the subscript, is numbered with the
  induction temporaries and takes the target's colour. 288 order and grouping
  cells floored at 9 on the pointer-pair shape; the index shape was 0 first try
  at delta 0. **Before sweeping any spelling axis on a walking pointer and its
  bound, ask which of the two the source declares.** The general form: where a
  residual is the colour ORDER of two values, check whether the target's source
  declares them at all, because a value the compiler creates and a value the
  source declares are numbered from different tables.
- **L151 confirmed on two more functions (2026-09-12)** — a literal's type is
  part of its IR identity, and it is the *literal's* type, not the variable's.
  Giving two otherwise identical `- 1`s different types splits a common
  subexpression that no carrier, order, scope or region edit splits; 96 cells
  over signed/unsigned/16-bit declared types of the same variables were
  byte-identical, so the declared type of a 32-bit local reaches nothing. Use it
  when the residual is "uopt commoned two expressions the target keeps apart",
  and price what the split costs before adopting: on both witnesses it bought
  the target's shape and lost one instruction elsewhere.
- **L146** — **a statement-order optimum belongs to the shape, not the
  function.** After any edit that changes the carrier shape, every recorded
  order sweep on that function is void and must be re-climbed. A 630-order
  exhaustion naming a unique optimum was overturned the day the shape changed,
  worth 98 → 26, and a second group then went 26 → 0. Conversely a plateau
  sitting behind an exhausted order sweep is a strong reopen candidate the
  moment a shape-changing edit lands.
- **L144** — `volatile` does *two* things: it makes every read a load from the
  value's home, and it emits scheduling edges pinning those loads in order.
  **Taking the value's address (`*(s32 *)&param`) does only the first.** With no
  edges a call-result copy ties with the reloads at `aftercycles` 0 and the
  block collapses to **ugen's emission order** — so where the target's order is
  ugen's, `volatile` overshoots and the address form lands exactly. A swept and
  failed `volatile` axis therefore does **not** retire the reload: one function
  had eight `volatile` forms measured flat and then **matched** on the address
  form. Ask which of the two effects the target's shape needs; on three siblings
  the address form failed for three different measured reasons.
- **L110** — uopt never merges an address constant across a basic-block
  boundary, and no read count produces a stack temporary. **Contested** — see
  L138; an address constant has been observed copied across a boundary, and
  preheader hoisting may explain it.
- **L105** — uopt forwards a call's return register into every use in the call's
  own block; move the assignment to a later block and it reaches the
  callee-saved copy instead.
- **L113** — a loop whose index dies at strength reduction emits no preheader
  `move`. If the target's preheader has none and yours does, give the loop an
  index of its own.
- **L109** — a discarded-expression probe's weight is its loop depth, so a save
  ratio is settable from source at zero instruction cost. Narrow: uopt counts
  after copy propagation, a probe reading an already-read global is CSE'd away,
  and only OR-with-zero, AND-with-minus-one and XOR-with-zero reliably count.
  Verify a probe moves `totalsave` in the records before building on it.
- **L107** — a uopt region boundary blocks address reassociation through a
  pointer's own definition, costing 8 bytes of temp, payable by shrinking a
  local aggregate. **L97** — `if (1) { }` or `do { } while (0)` opens a region;
  a bare `{ }` does not. **L111** — as1 fills a delay slot when the block holds
  a third node. **L59** — as1's tie-break reads physical source line numbers,
  last key LIFO; folding two statements onto one line retires a tie while
  swapping them only moves it. **L103** — the float constant pool is keyed on
  the constant's *spelling*, not its value. **L44** — carrier identity is the
  lever, not its presence. **L132** — a macro expansion carries the
  invocation's line number, so it is exactly one folded source line:
  rearranging statements *inside* a macro body is byte-inert, and "write it out
  longhand" is only ever worth what splitting those statements across lines is
  worth.

## Instruments

- `cc -S` — ugen's allocation before as1 schedules. Scratch cwd; `-o` is
  ignored and it writes `<base>.s` beside the input.
- `cc -Wa,-R` — as1's scheduler trace, byte-inert.
- `cc -g3` plus the `.mdebug` reader — exact frame home census.
**Scoring a forced object.** `score_symbol.py`, `align_symbol.py`, `frame_census.py` and `register_census.py` all recompile the TU with the *configured* command before measuring, which overwrites whatever object a `CDX_FORCE`d or instrumented run produced — so they report the **unforced** build. A lane read the same score for twelve different forces before noticing. They now warn when force or instrumented-compiler environment is set; score a forced object against its target directly instead.

- `uopt -Wo,-zdbug:2` — writes `./uoptlist` with the colouring decision records.
  **Never commit it.**
- **`DKWB_UGEN_SCHED=1 DKWB_UGEN_TRACE=1` — the freelist trace, and the most
  productive instrument of the campaign.** Each `ALLOC_GP_RESULT` /
  `ALLOC_FP_RESULT` row is stamped with its source line, so the draw index at
  any statement is a **direct readout**: "the target spends one more draw here"
  becomes arithmetic rather than inference. It was decisive on three of five
  targets in one lane and it is the only way to see an L149 folded draw. It also
  gives the free list itself — one integer list read `t6 t7 t8 t9 t1 t2 t3 t4
  t5` FIFO after `t0` was removed, and the **fp scratch ring is five registers,
  `f4 f6 f8 f10 f18`**, the complementary `f0 f2 f12 f14 f16` being globalcolor's
  colours 24–28. A "cycle" a census reports is not necessarily the free list's
  order; the trace settles it.
- The instrumented toolchain at `~/Desktop/dev/ido-instrumented` (`CDX_LOG`,
  `CDX_PROC`, `CDX_DETAIL_WEB`, `CDX_FORCE`). **Two usage facts that cost a lane
  time each:** `CDX_LOG=1` alone does nothing — the log path is **`CDX_OUT`**;
  and **`CDX_PROC` must be an ordinal**, which you get by first running with a
  non-numeric value to print the `procindex` table. **Confirm its `.text` is
  byte-identical to the tree's object before trusting any reading** — and
  **derive the compile command from the build rather than retyping it**. Take
  `nm_ranking.configured_compile_commands` and replace only the compiler binary.
  A hand-written line that drops a per-file flag makes *both* sides of the
  identity gate wrong in the same way, so they agree with each other and
  disagree with the tree: one TU's per-file `-Wab,-r4300_mul` produced 33
  against the configured 31, a gap small enough to read as noise. Then `cmp` the
  objects rather than trusting the gate's verdict alone.
- Direct `cc` equals the asm-processor build for a candidate with no
  `GLOBAL_ASM` pragma, at roughly 130 candidates/sec, which makes a small
  lattice exhaustive rather than sampled.

### The insertion-pair reader, for any function whose size is off

`tools/insertion_pairs.py <symbol>` is the instrument for a size-mismatch
function, and the one to run **before** any colour or draw work on it. Every
other instrument here works at size delta 0; they move a register, never an
instruction. This one answers the question a nonzero delta actually asks:
which word is extra or missing, and which construct of our source emitted it.

What it measures, all from one alignment (the same edit script
`residual_map.py` reads, so its offsets agree with that tool's by construction):

- **Pairs.** A pair opens at the first one-sided word and closes where the two
  streams are index-aligned again; one that never closes runs to the end. Two
  shifts that touch are two pairs.
- **Shadow**, per pair: positional masked words inside it less the aligned
  disagreement inside it. `aligned residual after shadow` is the positional
  count less every pair's shadow, which is exactly align_symbol's aligned
  disagreement. **That is the number to rank a size-mismatch function by**, not
  the masked count (L155). It also prints how much of that residual is
  register naming *inside* a pair -- the rows a free list rotated by the
  one-sided word would produce. Read that as an upper bound on what fixing the
  word could take with it, not as a measurement of it.
- **Class** of each one-sided word from its encoding (move, stack-load,
  stack-store, load, store, alu, const, branch, call, frame, delay-nop, nop).
- **Owner**: the source line from the candidate object's own `.mdebug` line
  table, and the ugen handler that emitted a word of that family on that line
  (`iloadistore` = ILOD/ISTR, `loadstore` = LOD/STR, `move_to_dest` = a copy,
  `jump` = FJP/TJP, `gen_reg_save_restore` = a callee save), read from the
  `DKWB_UGEN_TRACE` call stack. It compiles the trace itself and identity-gates
  it per function. Every owner states its **basis**: `line`, `prologue`,
  `nearest` (within three lines, because as1 schedules across statements),
  `as1` for a nop, or `neighbour` for a target-only word placed only by the
  line beside it -- the weakest. A word nothing owns says `unowned`.
- A **label** per pair from a fixed rule (`pair_label` states it): hoist,
  unrolled-loop, extra-ILOD, extra-ISTR, missing-CSE, split-not-copy,
  spill/reload, callee-save, control-flow, delay-slot, other, unowned.

What it cannot do. It reads **our** compile only: the target has no trace and
no line table, so a target-only word is owned by what our code does beside it.
The label names the one-sided word's class and owner, **not the edit that
removes it**. Measured: `func_800084C4` reads one extra ALU word on a line that
spells one subexpression twice, labelled missing-CSE; binding that
subexpression to a temporary left the object byte-identical, because uopt had
already shared it. The label tells you which word and which line; finding the
spelling is still a source edit judged by `draw_census.py --compare` and the
reader run again. It does not see uopt's decisions (a CSE, a hoist, an unroll)
except through the word they leave behind. `-g3` is not used for lines: it
changes the size of 30 of the 65 `objects.c` functions.

When to run it: first, on any function with `size_delta != 0`, and again after
every edit that moves a one-sided word. It refuses delta 0 (use
`residual_map.py`) and refuses a force or trace environment without
`--object`; score a forced object with `--object` and its own log with
`--trace`. `gmake small-delta-census` runs it over every |delta| <= 12 row
and writes `docs/small-delta-census.md`, sorted by aligned residual after
shadow -- the dispatch order for Track B.

## Traps that have each cost a lane real time

1. **Never read target words from splat's `.s` annotation** — that is the ROM's
   *linked* word. Assemble the listing and read the object.
2. **Check size delta before word count.** A stale object reads as a plausible
   coherent residual.
3. **A forced-colour receipt is numbered against its own compilation.**
   Re-derive any web number yourself and cite what you derived it from.
4. **`wb_compare.sh --summary-json` refuses whenever a linked
   `build/mickey.us.elf` exists** and blames your candidate. It is lying. Score
   before a full build.
5. **A lower positional count is not automatically closer once the schedule
   moves.** One candidate scored 37 against 39 while emitting five loads where
   the target emits three and filling both multiply-hazard `nop` slots the
   target leaves open. Count instructions too.
6. **The fast direct-compile loop needs the TU's own per-file flags.** Three TUs
   here carry overrides; a missing `-Wab,-r4300_mul` changes multiply expansion
   so the symbol scores as a *size* mismatch against a target it matches in
   shape. Check your harness's base score equals `score_symbol.py` once per TU.
7. **A harness that re-prints the source destroys physical-line levers.** Same
   check catches it.
8. **A carrier sweep ranked by score will offer a semantically wrong candidate
   that scores better.** One offered a loop's index carried by the x cursor
   inside that same loop, five words better than the correct answer; it would
   have compiled and verified against nothing. Require the candidate name to be
   absent from the region's text, then read the winner against the whole
   enclosing scope.
9. **A same-module call can still be a `SYMBOL` relocation record.** A function
   can score 0 masked words and still fail `verify` by exactly two, and
   `reloc_surface.py` says nothing. See `CLAUDE.md`.
10. **A per-file flag override is a falsifiable claim.** If the target holds an
    instruction the compiler cannot emit at that level — a branch-likely, or a
    rounding-mode float conversion — the override is refuted whatever it scored.
    `tools/check_isa_overrides.py` enforces this.
11. **Link failure on arrival:** run `gmake overlay-syms`, then build. If
    `git status` shows `overlay_undefined_syms.us.txt` turning a
    `<sym>_oNNReloc` back into a bare `<sym>`, that is lost-rename build drift —
    regenerate and rebuild, never commit that diff.
12. **Give yourself a private scratch subdirectory, outside the tree.** The
    session scratchpad is shared and two lanes have overwritten each other's
    files. Note that `mickey-lane-<name>` is a **symlink** to
    `mickey-lane-<name>.noindex`, so a directory you create as a "sibling" at
    `.../mickey-lane-<name>/foo` lands *inside the repository*.
13. **A debug dump's contents are evidence about the dump, not about the
    decision.** Two halves of one closure fell to this in one session. An
    `available` mask is written *after* the choice, recording what stays
    consistent with it — the colours it clears were mostly outbid, not
    forbidden, and the same build's cost list showed ten of them on offer at
    4.0 each. An empty `colorcand` bitset likewise does not mean globalcolor
    was idle; the instrumented records showed it colouring six webs. **Read the
    cost list and the decision records.** A pass is idle when its *records* are
    empty.
14. **Never pipe a gate's output.** `gmake check-docs 2>&1 | tail -3; echo
    "exit=$?"` reports *tail's* status, always 0, so a failing gate reads as a
    pass. Three times here, twice landing a commit on a red gate. Run
    `tools/gates.sh` (add `--staged` before a commit, `--promotion` after one),
    which keeps status and output apart and names every failure in a verdict.

## Rules, non-negotiable

- NEVER pass `--no-verify`. NEVER lower a check threshold. NEVER edit
  `config/lane-reopen-authorizations.us.json` — repinning is a coordinator
  action; if a stale pin blocks you, say so and stop.
- Nothing ROM-derived is ever committed: no assembly, no instruction text, no
  hexdumps, no machine-word arrays, no base64 of ROM bytes. `uoptlist` and
  compiler scratch are never committed.
- No absolute workstation paths in tracked files.
- A handoff shard is a strict grammar: the metric header must be source, score,
  frame, relocations, first mismatch, then an optional summary, each on its own
  line, and `|` is forbidden anywhere in the block — write measurements as prose
  or indented lists, never a markdown table.
- Before EVERY commit: `gmake verify` (must print the expected SHA1),
  `gmake cleanroom`, `gmake check-docs`. **Read the exit status, not the last
  lines of output** — filtering a gate until it looks clean is how a red gate
  gets committed.
- On a resident match: remove that function's `#ifdef NON_MATCHING` guard,
  `gmake extract`, rebuild, `gmake verify`, `gmake scoreboard`, and commit that
  function alone with a short comment at the point of use saying which edits got
  it there. On an overlay match follow the promotion sequence in `CLAUDE.md`,
  then `check-overlay-syms` and `promotion-proof`.
- Do not refresh `docs/nm-ranking.md` unless `check-docs` fails without it; the
  coordinator regenerates it. **A promotion is exactly that case**: matching a
  function retires its ranking row, so `check-docs` fails until the file is
  regenerated, and the lane that promoted it regenerates it. Two lanes read the
  sentence above as a blanket prohibition and handed back a tree that would not
  integrate; a third was told by its dispatch to regenerate unconditionally,
  which is the opposite error. Regenerate when, and only when, `check-docs`
  fails without it.
- **Regenerating means measuring.** `tools/nm_ranking.py` with no mode flag
  compiles and re-measures; `--write-doc` is documentation mode and does not
  compile, so it rewrites the Markdown from whatever the JSON already holds.
  Run the measuring form first, then `--write-doc`. Against a stale row,
  `--write-doc` alone is a no-op that looks like a success.
- **Your shard header is a claim, and it is checked.** `- score:` and
  `- first mismatch:` must agree with what the function measures when you hand
  back; `python3 tools/check_shard_metrics.py` compares every header against
  the ranking. `plateau_handoff_audit.py` does *not* catch this -- it compares
  a shard to its in-source marker, and `finalize_plateau.py` writes both
  together, so a drifted pair agrees with itself and reports `current`. On a
  match, say so in the header: a matched function leaves the ranking entirely
  and its shard stops being compared to anything, so a stale header there
  advertises an open residual forever. Sixteen shards were in that state on
  2026-09-16, one of them matched the same day.

## When your lane is merged

The coordinator removes integrated lane worktrees with
`tools/reclaim_worktrees.py --apply`, which keeps every branch and only deletes
checkouts that are not running, have nothing uncommitted, and carry no commit
missing from the integration branch. Each lane's `build/` tree is hundreds of
megabytes and the fleet does not shrink on its own -- one sweep reclaimed 31 GB
across 106 worktrees.

So **commit everything you want kept.** A worktree that reports clean is a
worktree whose findings live only in your report, and a report is not the tree.

## Reporting

Pair every claim with its measurement. Give the aligner's four buckets before
and after anything you adopt, not just the positional count. State explicitly
whether `gmake verify` ran on each claimed match.

For anything you do not close, leave an updated handoff naming the decision
variable you reached, the record that blocks it, and the axes your lattice
actually covered. A precise negative is a real result — several have been worth
more than a match, because the next lane inherits whichever you wrote down.

Where a prior closure turns out to be wrong, say so with the measurement that
breaks it. That is the most valuable thing you can report.
