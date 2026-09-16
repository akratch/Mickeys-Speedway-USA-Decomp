<!-- plateau-handoff:overlay1UpdateAimedTransient:start -->
### `overlay1UpdateAimedTransient` plateau handoff

- source: `src/overlays/o001/overlay_001_tail.c`
- score: 249/249 words
- frame: 0x80
- relocations: 43
- first mismatch: none
- summary: MATCHED. This header claimed an open residual until 2026-09-16, when tools/check_shard_metrics.py reconciled every shard against the ranking; the function had already left the queue. The claim it carried was: 473-draw census confirms closed prologue-load emission-order blocker; no source-authentic route below 14.
- assignment base: `ab2e28755e75281263cff6b4846893469a252f61`
- owned range: Overlay 1 `+0x6D4C..+0x7130`, 996 bytes / 249 instructions, with no size delta
- baseline: 64 raw differing words and 57 positional differences, 39 opcode mismatches, eight alignment gaps, and frame `0x88` versus target `0x80`; the runtime surface had 43 target records versus 45 candidate records, with 30 offset/type positions aligned
- retained improvement: Carrying the shared-world address as an O32 `u32` keeps its two dereferences in the target saved-register lifetime and removes the candidate's duplicated address materialization. Moving `savedState` before `state` then aligns two stack-home constants. Together these changes reduce the raw residual from 64 to 32 words and the positional residual from 57 to 23.
- current comparison: Exact 996-byte extent and 249 instructions, 32 raw differing words, 23 positional differences, 15 opcode mismatches, and eight alignment gaps. The frame remains `0x88` versus target `0x80`; the first mismatch is the frame adjustment at `+0x0`.
- relocation proof: Target and candidate now both carry 43 runtime records. A diagnostic comparison aligns 38/43 offsets and types and 33/43 stable identities, with 34 candidate identities resolved. Strict `function_preflight.py` still fails closed because shared-TU aliases including `overlay1ReadSelection` have ambiguous runtime identities; these diagnostic counts are not an exact relocation claim.
- attempts: Ten coherent forms were compiled: two constant-load lifetime forms, `register`, generic `void **`, volatile-pointee and declaration-initializer world references, an O32 integer address carrier, initialized address/world declarations, and saved-state declaration order. The volatile and constant-lifetime forms regressed structure; three reference spellings were byte-flat. The O32 carrier and declaration order are the only retained gains.
- flags: The 119-case lattice completed with the configured `-O2 -mips2 -32 -Wab,-r4300_mul` row best at 23 positional differences; `-g3` regressed to 25. No global flag change is supported.
- mechanism: `decomp-workbench` reports `structure-mismatch`. FP pool and temporary lanes are exact; the general temporary lane is exact and the general pool first differs at one early world/object load. The remaining source-visible blocks are the early address/load placement, one stack home, and the no-source motion constant load around the first angle call.
- next action: Reopen only with a source-authentic typed declaration for the shared-world symbol, evidence for the original stack-home lifetime, or an authenticated constant/angle-call spelling. Do not repeat these ten forms or run an unbounded generic permutation search.
- DKR/JFG: The exact atlas route has no function-sized row for this old mixed-TU fallback, so a read-only raw skeleton comparison was used. Its nearest DKR and JFG shapes are only 0.0308 and 0.0303; coddog found no exact chunk. JFG's assembly-only `src/overlays/o95/overlay_95.c::flamethrowerConstrain` is the nearest size-compatible row (980 bytes, 0.0303), while `src/overlays/o113/overlay_113.c::missileControl` and `src/overlays/o67/overlay_67.c::mantismissileControl` score 0.0078 and 0.0119. The Mickey candidate may illustrate a generic three-pass intercept estimate, but it is not a donor or public-ledger-grade JFG lead.


#### tu2-o1tail: two frame identities proved, and the address carrier is load-bearing

The 8-byte frame error was a declaration, not an allocation: the local list
carried `Overlay1TransientOwner *owner`, declared and never used. Deleting it
takes the frame from -136 to the target's -128 exactly. With the frame right,
the one remaining stack home was 4 bytes low (sp+112 against the target's
sp+116); moving `savedState` one position up the declaration list puts it on
the target's slot. Both are identities, not guesses: the frame size and the
home offset each match the target now, and the residual fell 23 to 21 words.

The `u32 worldAddress` carrier is load-bearing and must stay. Removing it and
reading `D_1DA0` directly costs 36 words (21 to 57) and 8 bytes of frame,
because the saved-register carrier for the world address disappears with it.

Residual, 21 words. The largest piece is at +0xC: the target computes the low
half of the D_1DA0 address and loads the world pointer immediately after the
`lui`, in among the register saves, while the candidate defers both until
after the saves. The rest is the trig load's position in the sourceless
branch, where the target loads it after the first call and the candidate
before. Eliminated, do not repeat: reading `overlay1AimedTrigReloc` directly at
both use sites (a call sits between them, so it is reloaded: +2 instructions,
87 words); an embedded assignment inside the multiply expression (+1
instruction, 86 words); folding the address and dereference into one statement
(flat); and a pointer-to-pointer carrier in place of the u32 (53 words).

#### lm-o1tail: both clusters are schedule, and the trig one is now reachable

Re-measured at the assigned base: 21 masked words, 249 of 249 instructions,
the 0x80 frame and the stack home exact, 43 relocation records. The residual is
exactly two clusters and both are placement, not allocation. Every register in
the function already matches.

Cluster one, fourteen words at +0xC: the target computes the low half of the
shared-world address and dereferences it *among* the register saves -- `sw s0`,
`lui s0`, `addiu s0`, `lw`, then `sw ra`/`sw s3`/`sw s2`/`sw s1` and the six
`sdc1` pairs -- while the candidate emits the same `lui` in the same slot and
defers the `addiu` and the load until after all eleven saves. The address
carrier itself is right: `s0` holds the address for exactly two dereferences
and is then reused as the loop counter, in both.

Eliminated for cluster one, do not repeat: five address-carrier types (`u32`,
`s32`, `register u32`, `u8 *`, `void *`); four initialisation spellings
including a declaration initializer and a `u32 *` double dereference; three
orders and three line groupings of the first four statements; the full
carrier-versus-direct matrix over all six shared-world access sites, all eight
combinations (only the all-carrier form keeps 249 instructions and the 0x80
frame; the all-direct form hoists the load above every save but loses the
`s0` carrier, 53 words); an `Overlay1TransientWorld **` typed carrier and a
bare pointer variable, both of which IDO folds straight back into a two
instruction `lui`/`lw` and lose the carrier; the `D_1DA0_array[0]` array
declaration idiom this TU already uses elsewhere, in three forms; and **306
single-declaration-position moves over the entire eighteen-declaration list**,
every one of which is byte-flat. Declaration order is inert for this function;
statement order and spelling are exhausted. What is left is the scheduler's
own priority between a load and the prologue stores.

Cluster two, seven words at +0x2CC, is now reachable. The target loads the
trig constant *after* the first angle call, using the call's delay slot for
the owner-angle load; the candidate hoists the load above the call because the
`trig = overlay1AimedTrigReloc;` statement precedes it in source order. Folding
the assignment into the multiply -- `func(angle) * (trig = overlay1AimedTrigReloc)
* -30.0f` -- reproduces the target's `jal`, its delay slot, and the post-call
load order exactly. It costs one instruction: a stall `nop` between the loaded
constant and the multiply, which the target fills with the two address
materialisations of the following statement. That is the whole remaining gap on
this cluster, and it is a scheduling fill, not a source form. Ten spellings of
the embedded form were measured (parenthesised, negated, joined onto one line,
assigned in the second use instead of the first, three different existing `f32`
locals as the carrier, and the velocity-Y statement moved first); all are 250
instructions. Reading the global directly at both sites is 251.

Assigning the call result to any existing `f32` local before reading trig also
places the load after the call, but costs a `mov.s` for the same net 250.

Next lever: the `nop` fill, which needs the two following `%hi` materialisations
scheduled into it -- an ordering question about the statement *after* the one
being fixed, not about the trig statement itself.


#### c2-o001: both clusters are decided above `lineno`, and the assembler prints it

Re-measured at 19 masked words, 249 of 249 instructions, the 0x80 frame and
the stack home exact. `cc -Wa,-R` was read for both clusters and neither is
reachable from C by any placement, spelling or ordering lever. This is the
reachability test L79 describes, run on this function's two blocks.

Cluster one, the fourteen words at +0xC, is not a scheduling residual at all.
In the prologue block as1 gives the frame adjustment 19 aftercycles, the saved
low-register store 10, the address-high node 9, the address-low node 8, and
every one of the ten remaining register saves 8 as well. The scheduler picks
the frame adjustment, then the store on 10, then the address-high on 9, and at
the next step the address-low ties the saves at 8 and loses the tie on
emission index -- the saves are emitted at indices 3 to 13 and the address-low
at 18. The tie is structural, not incidental: every save carries a dependence
edge to the world dereference, so their aftercycles is one plus the
dereference's, and the address-low's aftercycles is one plus the same
dereference's. They can never separate.

The edge is the point. as1 orders every stack store in the block before every
later load in it, so the dereference cannot rise above the save block no
matter what its priority is. The target's arrangement, with the dereference
sitting among the saves, therefore cannot be produced by scheduling ugen's
emission order at all; it requires ugen to emit the dereference before the
saves. Nothing on this page's lever set reaches that, and the eighteen further
spellings measured this lane did not either.

Cluster two moved and then closed. The remaining five words are the order of
two `at`-based materialisations after the first angle call: the target loads
the trig import and then materialises the -30.0f float immediate, and the
candidate does the reverse, which also reverses both multiply operand orders.
Writing `trig = overlay1AimedTrigReloc;` as its own statement fixes the
multiply operand orders exactly, at 249 instructions and 21 words, but hoists
the load above the call; assigning the call result to any existing f32 local
first puts the load back below the call and keeps the operand orders, at 250
instructions with one stall.

In every 250-instruction form the as1 trace shows the float immediate emitted
at instruction index 2 of the post-call block and the import load at index 4,
with the load carrying two predecessor edges on the shared `at` register. The
two pairs are hard-serialised by `at`, so their order is ugen's emission order
and the scheduler has no freedom in it. Twenty forms were measured this lane
and every one of them emitted the immediate first: the call result carried in
five different existing f32 locals; the -30.0f constant carried in four
different existing f32 locals, placed before and after the trig statement; the
velocity-Y statement hoisted above velocity-X; a volatile-qualified read of
the trig import; the trig-left and fully-parenthesised right-grouped products;
the trig import folded into the call-result statement; and a `-30.0f` folded
into the trig variable itself. Best of them is 250 instructions; the retained
249-instruction candidate stays.

Reopen only with evidence that moves ugen's emission order, not as1's
selection: a source form that emits a global load before the prologue saves,
or one that emits a float import load before a float immediate in the same
block. Do not spend another lane on placement, grouping or declaration
lattices here -- three lanes have now closed those, and this one closed the
mechanism behind them.
#### 2026-09-11, lane f9-audit: the trig cluster falls to a region boundary, 19 -> 14; the prologue cluster re-read

Measured with the direct `cc` loop (the TU's own `-Wab,-r4300_mul` row; without
it the direct object is four instructions short, so check the flag before
trusting any score here) against the ranking's comparator. Base reproduced at
19 masked, 249 of 249 instructions, frame `0x80`, delta 0.

**Cluster two does not stand, and the variable it held fixed was block
membership of the constant's definition, not ugen emission order.** uopt hoists
a CSE'd float constant to the head of the first block that uses it, so in every
form where the trig load and the multiply share a block the `-30.0f` immediate
is emitted ahead of the load, whatever the statement order. Read off the
objects: the three-statement form (call result into an existing f32 local,
`trig = ...;` next, the product third) already gives the target's operand
order and the post-call load, and costs exactly one stall `nop` -- no `mov.s`,
contrary to the previous lane's note -- because the immediate is still first.
Opening an L97 region after the trig load, with all three velocity stores
inside, moves the block head below the load: **14 masked, delta 0, and the
whole else arm is exact.**

Measured on that form, all delta 0 unless noted:

- carrier for the call result: `factor`, `distance`, `predictedX` 14; `deltaY`
  16, `deltaZ` 17, `predictedY` 20, `predictedZ` 23, `deltaX` 25 -- carrier
  identity is the lever again (L44) and the object cannot say which of the
  three tied names is original;
- `do { } while (0)` ties `if (1)`; a bare block is +1 instruction (the L97
  control);
- region extent: velocityX alone inside, the trig load inside, the call inside,
  or the whole else arm inside are all +1 instruction; only "load outside,
  three stores inside" reaches 14;
- the assignment-expression forms `call * (trig = X) * -30.0f` and the comma
  form `(trig = X, trig)` are +1 and also flip the first multiply's operand
  order, so the `(trig = X)` operand carries a different L92 weight from a plain
  local read -- which is why the retained 19-word form had the operand order
  wrong at both multiplies;
- `-(30.0f)` and `(0.0f - 30.0f)` are byte-identical to `-30.0f`; negating the
  call instead is +3;
- `extern const f32` for the trig import, and a `static` file-scope float, are
  both reloaded across the call (+2 instructions): IDO 5.3 does not treat
  either as call-invariant, so the single load in the target needs a local.

**Cluster one, the 14 words, re-read against the objects rather than the as1
trace.** The closure said as1 orders every prologue store before every later
load and therefore ugen would have to emit the dereference before the saves.
The all-direct form (51 words) shows otherwise: its first `D_1DA0` read is
scheduled at `+0x4`, above every save, so a symbol-class load has no
store-to-load edge at all. What holds the candidate's load below the saves is
its memory class -- an indirect load through an opaque integer carrier -- and
what the target has is a symbol-class load whose address is nevertheless kept
in `s0` for the store site. No spelling measured has both:

- opaque carriers keep the sharing and the class: `u32` (19 then), a typed
  pointer assigned through `(u32)`, a volatile pointee, a `[0]` subscript --
  all byte-identical to the base;
- typed carriers fold to two separate symbol loads and lose the sharing: a
  plain `Overlay1TransientWorld **`, a one-element array decay, a struct-holder
  pointer -- all 51, the same object as reading the global directly at both
  sites;
- mixed forms (first read direct, store through the carrier; or the reverse; or
  a second direct read of `source` in the head block) each cost +1 instruction
  because the address is then materialised twice;
- a region boundary anywhere in the head block moves the frame (delta -16 or a
  changed frame word), so L97 does not reach this cluster.

The census for L108: 310 p1 records, zero p2, so definition order (L106) is
not an axis for either cluster.

**Next lever for the 14 words, stated as the requirement:** a source form in
which uopt shares the `&D_1DA0` materialisation between the head-block read and
the store inside `if (object == 0)` while the read keeps its symbol memory
class. L110 says uopt does not merge an address constant across a basic-block
boundary, and every typed form here confirms it; the target did it anyway, so
either the original's store site is in the head block's region in a way this
reconstruction does not reproduce, or the sharing comes from a construct that
is not a plain local. Do not spend another pass on carrier types, declaration
order, or statement order in the head block -- this pass and the three before
it have closed those on the same objects.
#### 2026-09-11, lane p6-mid: the prologue cluster is a structural tie in as1's ready list, read off the node graph

Baseline reproduces: 996 bytes, 249 of 249 instructions, delta 0, 14 masked
words, first mismatch +0xC. Aligner on a register-erased shape: 244 byte-exact,
1 register naming, 2 immediate-only, 4 really different, displacement tax 7 --
so the aligned disagreement is **seven words**, all of them the prologue
cluster, and the other seven of the positional count are pure shift.

The f9-audit closure read the mechanism as memory class and was right. This pass
prices it from `cc -Wa,-R`'s own dependence graph for the entry block, which
turns "the candidate's load is held below the saves" into a statement about why
no source form can lift it.

**The graph, with the numbers.** ugen emits the frame adjustment, then eleven
register saves, then the address materialisation as a single `la`, then the
world dereference and the two field loads. as1 expands the `la` into two nodes
before it builds the graph. Their priorities:

- frame adjustment, aftercycles 19;
- the save of the address carrier, aftercycles 10 -- it carries an
  anti-dependence to the address-high node, which is why as1 already hoists the
  address-high into the middle of the save block in the candidate;
- address-high, aftercycles 9;
- **address-low, aftercycles 8**;
- **each of the other ten saves, aftercycles 8**, and each lists the same three
  afternodes: the world dereference and the two field loads;
- the world dereference, aftercycles 7; the object field load, 4; the source
  field load, 0.

as1 selects on aftercycles and breaks ties on emission index, which the trace
shows directly: the ten saves are emitted at indices 3 to 13 and the address-low
at 18, and the pick order runs 3, 4, 5, 6, 8, 9, 10, 11, 12, 13 and only then
18. The address-low can never win that tie.

**And the tie is structural, not incidental.** Every save's aftercycles is
`1 + aftercycles(world dereference)` because each one carries an ordering edge
to it; the address-low's is `1 + aftercycles(world dereference)` because its
only afternode is that same load. The two terms are the same expression. Any
edit that lengthens or shortens the chain after the load moves both by the same
amount, and any edit that adds a deeper load to the block raises the saves
without raising the address-low. **There is no source form that wins this tie
while the edge exists.**

**The edge exists exactly when the load carries no relocation, and that is
measured, not assumed.** Compiling the all-direct form and reading the same
block: its world dereference is emitted as a symbol reference, its node records
`before 1` -- the address-high, and nothing else -- and every save drops to
aftercycles 5 with only the two field loads as afternodes. The candidate's
node records `before 12`: eleven saves plus the address-low. So as1 disambiguates
a stack store against a memory reference that names a static symbol, and against
nothing else; a register-indirect load gets an ordering edge from every
preceding store in the block no matter what its base register was just built
from.

**Which is why the requirement is now a contradiction rather than a search.**
The target's own object was checked: its relocation surface carries a HI16 and a
LO16 for the shared-world symbol at +0x8 and +0xC and **nothing at +0x10**, so
its world dereference is register-indirect and reloc-free, exactly like the
candidate's -- and it still sits above ten saves. With ugen's emission order
fixed (all eleven saves before the first body instruction, confirmed in `cc -S`)
and as1's tie unwinnable, the shipped arrangement cannot be produced from this
emission order at all. Either the original's saves were not all emitted first,
or the load was a symbol reference that the extraction cannot show. Neither is
reachable by carrier type, spelling, statement order, declaration order or a
region boundary, and four lanes have now exhausted those.

**Do not spend another pass on the head block.** What would settle it is
evidence about ugen's emission order rather than about as1's selection: a
function anywhere in this tree whose ugen output puts a body instruction between
two prologue saves. One such example turns this from a contradiction into a
lever; without one the seven words are the price of the reconstruction.
#### 2026-09-12, lane `p10-near`: L145 and L59 both tested on the prologue cluster, both flat

Baseline reproduces: 996 bytes, 249 of 249 instructions, delta 0, 14 masked,
first mismatch +0xC. Aligner: 244 byte-exact, 1 register naming, 2
immediate-only, 4 really different, displacement tax 7.

Two laws that landed after the previous closure were tested against it, because
both change what ugen emits rather than how as1 selects, which is what the
closure asked for.

**L145 -- delete the carrier -- does not reach it.** Writing the shared-world
dereference as the expression itself at both sites, with no `worldAddress`
local at all, scores 18 in three spellings (`(u32)&D_1DA0`, `(s32)&D_1DA0`,
`(u32)(&D_1DA0)`) and 50 with a plain `&D_1DA0`. All are size delta 0 and 249
instructions, and the aligner shows the structural residual **unchanged**: the
same two surplus words at +0x34 and +0x40 and the same two missing at +0xC. What
the deletion does change is the frame: four immediate-only words appear,
including the frame adjustment itself. So the no-carrier form is a different
home set with the same schedule, not a different schedule.

**L59 is inert on this block.** Eight physical-line arrangements of the first
four statements -- all four on one line, each adjacent pair folded, the first
statement moved onto the last declaration's line, four blank lines inserted
ahead of them, and the address folded into the declaration as an initialiser --
are **all byte-identical at 14**. That is consistent with the previous lane's
node-graph reading: the tie as1 breaks here is between prologue stores and the
address-low node, and prologue stores carry no source line for `lineno` to
order.

The closure therefore stands as p6-mid wrote it, with its evidence requirement
unchanged: a counterexample to ugen's emission order, not another spelling.
Both of the two levers that could plausibly have supplied one are now measured
and negative.

### 2026-09-12, p23-lastmile2: exhaustive colour survey confirms displaced words

The configured baseline remains 996 bytes, 235/249 masked words exact,
frame 0x80, 43 relocations and first positional mismatch +0xC. The aligned
split is 244 exact, one naming and two immediate rows, with no paired
structural rows, but two candidate-only words at +0x34 and +0x40 and two
target-only words at +0xC and +0x10. Equal total size therefore does not
establish equal instruction shape. The earlier four structural rows counted
these unmatched words; positional comparisons also charge the displaced span.

The exhaustive same-kind colour plan covered all 26 coloured webs: 154 plan
entries, including one with no alternative, and 153 actual force probes.
150 were accepted and three declined (web 196 colours 5 and 6, web 210 colour
6). Of the accepted probes, 75 retained size and 75 changed it. The complete
improving winners list is empty. The measured single-force and empty packing
floor is 14; this is not a proof about arbitrary combinations of regressing
forces. No diagnostic force is source or matching evidence.

The named source question remains whether ugen can emit the shared-world
load with symbolic memory scheduling while retaining shared address materialization.
The prologue save/load ordering creates the unmatched words; recolouring cannot
remove them. The prior carrier, region, declaration and physical-line experiments
already eliminate the available source mechanisms, so no new source sweep was
justified. The guarded candidate is unchanged apart from plateau metadata.

Private artifacts under build/p23/overlay1UpdateAimedTransient retain the
configured baseline, trace mapping, aligned map and all force measurements.
Commands: configured stock/capture compilation, residual_map.py,
web_footprint.py --every-colour, and finalize_plateau.py. The matching claim
remains NON_MATCHING; a ROM verify of the guarded fallback gives no new bytes.

<!-- plateau-handoff:overlay1UpdateAimedTransient:end -->
