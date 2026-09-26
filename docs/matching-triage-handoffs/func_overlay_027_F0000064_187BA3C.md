<!-- plateau-handoff:func_overlay_027_F0000064_187BA3C:start -->
### `func_overlay_027_F0000064_187BA3C` plateau handoff

- source: `src/overlays/o027/overlay_027.c`
- score: 48 differing words
- frame: 0x60
- relocations: 22
- first mismatch: +0x94
- summary: Shorten scale live range off the call-argument register failed: fresh local +4/272, field recompute +28/333, block temp +4/272. No delta-0 masked drop; stall.

Summary before this remeasure: Required call-input carrier reuse is byte-inert; fraction reuse adds structural residual. The real-ABI no-spill source obstacle remains.
#### Region partition (measured at 62, re-measured at 48)

At 62 the 368-word residual partitioned into four independent decisions, not
one. Counts are relocation-masked differing words:

- 3 words -- stack home of the update-rate copy: which frame slot the
  retained copy of `updateRate` gets (+0x18, +0x47C, +0x480). **closed**
- 11 words -- post-loop fade and pulse-state webs: which colour the fade read
  and the pulse-state read take. **closed**
- 4 words -- in-loop value webs: the case-3 intensity read sharing a web with
  the other cases. **closed**
- 40 words -- persistent fp web: the register the long-lived scale value gets,
  f12 in the candidate and f16 in the target. open
- 4 words -- instruction placement: two adjacent-pair orderings, one around
  the default-case scale load and one in a call's argument setup. Two of the
  four fell out with the fade web; open.

Three closures, all delta 0 and confirmed on both the positional count and the
schedule-tolerant multiset distance:

1. Declaring the update-rate copy **first** in the local list moves its frame
   slot up by 8 and closes all three stack-home words (62 -> 59, multiset
   49). This is the L99 declaration-order axis, and only the first position
   works: positions 1-8 measured 62, 62, 64, 64, 64, 66, 66, 66.
2. Caching `state->fade` into the existing `value` local across the post-loop
   null test and the 0xFF test closes 7 (59 -> 52, multiset 44). The two reads
   then share one web and that web takes v0, which is what the target does.
3. Using the already-declared `phase` instead of `value` for the case-3
   intensity read closes 4 more (52 -> 48, multiset 41).

#### The open 40: one fp colour, and what it is not

Every one of the 40 is downstream of a single choice. The candidate gives the
long-lived scale value f12 (an fp argument register, 9 uses); the target gives
it f16 and uses f12 only once, for the one outgoing float argument. The 1.0f
constant is f14 in both, and both re-materialise the scale value after every
call, so neither web crosses a call and neither needs a spill. With the scale
value on f12 the candidate's ring temp pool is {f4,f6,f8,f10,f16,f18}; the
target's is {f4,f6,f8,f10,f18}. That one-element difference rotates the ring
at every arithmetic site in the switch, which is where the other 39 come from.

Measured flat on this axis (no change to the fp assignment at all):

- 18 declaration-order forms for the two fp locals (positions 0-8 each).
- A 64-cell lattice over six commutativity/statement-order axes: the order of
  the three coordinate copies, both scale multiply operand orders, the
  scale-target multiply order, the two fade-float multiply orders, and folding
  the pre-loop scale assignment onto the enclosing `if` line (L59). All 64
  cells measured 59-60.
- Loop form (`do`/`for`/`while`), a `register` qualifier, an extra local
  carrying the call's float argument, and a copy-through second local.

What does move it, and why it cannot be adopted as it stands: making the scale
web live across the call -- either by assigning it before the call in case 0,
or by giving case 0 its own scale local -- moves it off f12 immediately and
brings the whole ring into agreement (multiset distance 49 -> 37, the lowest
measured). Both forms cost a spill pair around the call, +8 bytes, so the size
delta stops being 0. The next attempt should look for a spelling that denies
f12 without extending the web's lifetime across the call.

#### Cross-function note

`func_overlay_079_F0000134_18CD0D4` has the same signature: its candidate uses
f12 eleven times more than its target and f16 nine times fewer. The two
functions plausibly share one mechanism, so a solution here transfers.
#### Further forms measured flat at 48

- Switch case order: 23 permutations that keep `default` last. All scored 75
  or worse; the source order is already the target's.
- The eight remaining integer words are the default arm's fade and intensity
  reads. Caching the intensity read into the existing `value` local is exactly
  flat (48, multiset 41); into `phase` it costs a size delta; splitting the
  fade subtraction, moving the cache below the test, and caching the case-3
  test operand all measured 49 to 94. Adding a dedicated local regresses by
  eleven words because it moves every frame slot, the same frame sensitivity
  overlay 79 shows.
#### 2026-09-11, lane p6-mid: the fp colour is proved causal, and the requirement is now arithmetic

Baseline reproduces: 1472 bytes, 368 of 368 instructions, delta 0, 48 masked
words. Aligner on a register-erased shape: 320 byte-exact, 46 register naming,
0 immediate-only, 2 really different. So 46 of the 48 are one bucket, and this
pass names the decision behind them instead of describing it.

**The whole residual bar nine words is one colour, and a force proves it.**
The instrumented `uopt` reproduces the tree's object exactly -- same 48 masked
words at delta 0 with logging on, which is the identity gate -- and the scale
carrier is procedure ordinal 1's web 35, class 2, `save` 7.100000, `nocs` 10,
`totalsave` 71.000000. Forcing it one colour up:

    CDX_FORCE=p1:w35=c28   ->   9 masked, delta 0
                                359 byte-exact, 7 naming, 0 immediate, 2 different

That is 320 -> 359 byte-exact and 46 -> 7 naming rows from a single allocator
decision, scored on the forced object directly rather than through
`score_symbol.py` (which recompiles with the configured command and would have
reported the unforced 48).

**The fp colour table for this procedure, decoded from its own `p1cost` rows.**
Twelve colours: c24 through c29 `kind=caller`, c30 through c35 `kind=callee`
at a flat cost of 20.250000 each -- six caller-saved and six callee-saved
even-numbered singles, which is the whole o32 float file. c26 is the colour the
candidate gives the scale carrier, and the register identity is settled by
force-and-diff rather than assumed: forcing web 35 off c26 takes the number of
sites naming the fp argument register from nine to exactly one, and that one is
the outgoing float argument -- the target's own shape.

**The decision ladder, and therefore the requirement.** Only four fp webs are
offered a colour at all; every other one splits at `bestcost` 20. In descending
`save`:

- web 120, `save` 30.000000, `nocs` 1, `totalsave` 30.000000, takes c24;
- web 64, `save` 30.000000, `nocs` 1, `totalsave` 30.000000, takes c25;
- web 35, the scale carrier, `save` 7.100000, arrives with `forbidden0`
  0x000000c0 and `available0` 0x0000003c, and takes the first available, c26;
- web 257, the 1.0f constant, `save` 5.454545, `nocs` 11, `totalsave`
  60.000000, takes c27.

So the previous closure's "deny f12 without extending the web's lifetime across
the call" is now a number: **web 35 needs both c26 and c27 forbidden at its
decision, which means two further fp webs coloured before it -- `save` strictly
above 7.100000 -- that interfere with it.** Raising web 257 above 7.1 supplies
only one of the two: 71/10 against 60/11 means web 257 needs `totalsave` 79 or
above at its present `nocs`, or `nocs` 7 or below at its present total, and
even then web 35 lands on c27 rather than c28. The arithmetic also kills the
mirror-image move, lowering web 35: dropping it below 5.454545 puts web 257 on
c26 and web 35 on c27, which the force table scores at 25, not 9.

**Second force round, all flat.** With `p1:w35=c28` as a prefix, forcing webs
120, 64, 257, 264, 255, 258, 260 and 261 to every colour the ladder offers
leaves 9 masked (the three that keep delta 0) or costs 8 to 20 bytes. The nine
that remain are a different bucket entirely: six naming rows where one integer
web reads a0 against the target's v0, one two-row schedule swap of an immediate
against a float load, and one commutative operand order on a compare. Forcing
each integer web that holds a0 (79, 143, 186, 226, 250, 253) onto c1 is flat at
9 or worse, so that web has not been identified yet and is the next thing to
name.

**Newly falsified, each at delta 0 and each measured against the whole
368-instruction target.** The constant-carrier family does not create an fp web
at all -- exactly the coalescing result overlay 71 recorded for its flag
chains. Hoisting 1.0f into a fresh `f32` local defined before the loop, the same
for 32.0f, both together, and each with the declaration first or last in the
list: every one is 46 naming rows, identical to the base, and the only thing
that moves is 9 to 11 immediate-only words as the stack homes shift. Carrying
1.0f in the already-declared `fraction` is +20 bytes.

Also falsified, and these were the untried structural axis rather than a
respelling: a `f32 *` pointer carrier for the scale in place of the `f32` local,
with the three assignments becoming address-of and the five uses becoming
dereferences (+16 bytes, 366 words); an `s32` index carrier over the four
adjacent float globals with the uses subscripting from the first of them (+56
bytes, 378); reading the ease input through that same block base (byte-identical
to the base); `volatile` on the scale (+20 bytes); `register` on the scale or on
`fraction` (both byte-identical); `f64` for the scale (+88 bytes); carrying the
outgoing float argument in `fraction` or in the scale local itself (both
byte-identical -- uopt propagates the global load straight through); splitting
the call out of the subtraction (byte-identical); and deleting the pre-loop
scale assignment (210 words at delta 0).

The known +8-byte control still reproduces and is worth keeping on the page
because it bounds the prize: assigning the scale before the call in case 0 takes
naming from 46 to 20 and costs the spill pair.

**Reopen condition.** A source form that creates an fp web with `save` strictly
above 7.100000 which interferes with the scale carrier, or two of them. Every
carrier spelling tried so far coalesces into an existing constant web instead of
becoming a new one, so the form has to be one uopt cannot fold -- and note that
the two webs already above the scale carrier both have `nocs` 1 with
`totalsave` 30, which is the shape to reproduce, not a long-lived value.
##### The fp colour table decoded to registers, and the four webs named

Forcing the scale carrier onto each colour in turn and reading the register it
lands on settles the mapping without guessing: c26 f12, c27 f14, c28 f16,
c29 f18, and by descending order c24 f8 and c25 f10. **So this procedure's
floating-point colour table is f8, f10, f12, f14, f16, f18 for the caller-saved
half and f20 through f30 for the callee-saved half -- and f0, f2, f4 and f6 are
not in it at all.** They appear all over the residual because they are pure ring
temporaries. That is L130 for the float bank: no declaration, carrier, qualifier
or spelling can put a value in f0, f2, f4 or f6 here, and any lattice that tries
is refuted as a class before it is run. Worth checking on a second procedure
before it is written up as a law, but it is a one-command decode.

The four webs that take a colour, identified the same way -- force each onto a
callee-saved colour and read which value moves:

- web 120, c24 f8: the case-1 fraction, defined from the timer division and read
  twice, once into the state's float field and once for the byte scale. Three
  references at loop depth 1 and one component, so `totalsave` 30 and `nocs` 1.
- web 64, c25 f10: the case-0 read of the state's scale target, three references
  at loop depth 1 against one component, the same 30 over 1.
- web 35, c26 f12: the scale carrier itself, `totalsave` 71 over `nocs` 10.
  71 is seven references at loop depth 1 plus the one before the loop.
- web 257, c27 f14: the 1.0f constant, `totalsave` 60 over `nocs` 11.

Which makes the shape to reproduce concrete rather than abstract: the two webs
that outrank the scale carrier are **short-lived expression values with exactly
three references inside the loop and a single live-range component**, not
long-lived carriers. A fourth such value, plus a fifth, coloured before the scale
carrier, is the whole requirement. Note the threshold for becoming a web at all:
the case-0 fraction has only two references and never becomes one -- it stays a
ring temporary -- so three references at loop depth 1 is the floor, and that is
why every constant-carrier form measured this pass left the fp web set untouched.
##### Corrected the same pass: the requirement is TWO conditions, and neither alone reads as progress

The "two more fp webs above the scale carrier" reading above is wrong, and the
records say why. Read web 257's mask in the forced run: with the scale carrier
moved to c28, the 1.0f web still comes back with `forbidden0` 0x000000e8, so
**c26 is forbidden to it independently of anything the scale carrier does**. The
1.0f constant is live across the one call that takes a float argument, and the
argument register is f12, so f12 is intrinsically out of reach for it. The scale
carrier is not live across that call, which is the entire difference between
them. A direct force of the 1.0f web onto c26 is declined, byte-identical, and
proves nothing -- L101's third kind.

In the forced object that scores 9, **f12 is named at exactly one site**, the
outgoing float argument. Nothing in the colour table holds it. So the target
does not reach f16 by stacking two more webs on top; it reaches it because f12
is forbidden and f14 is already taken:

- **Condition A -- the scale carrier must be live across the float-argument
  call**, which puts c26 in its forbidden mask exactly as it is in the 1.0f
  web's. Measured: moving the case-0 scale assignment above that call takes
  web 35's `forbidden0` from 0x000000c0 to 0x000000e0 and its `save` from
  7.100000 to 6.454545 at `nocs` 11. It also costs a spill pair, +8 bytes.
- **Condition B -- the 1.0f web must be coloured before the scale carrier**, so
  that c27 is taken and interfering when the scale carrier decides. Under
  condition A that is `save` 5.454545 against 6.454545, so B is **not** met and
  the scale carrier lands on c27 f14, not c28 f16.

That is why condition A alone reads as a failure: it is +8 bytes for 46 -> 20
naming rows and a worse positional score, and three lanes have now measured it
and set it aside. Forcing both on the A-form -- `p1:w35=c28,p1:w257=c27` --
gives **6 naming rows and 325 byte-exact**, with the +8 bytes and the schedule
shift behind it the only things left. So A and B together are the complete
colour requirement and nothing else in the fp bank is wrong.

**What each condition needs, arithmetically.** B needs `save`(1.0f) above
`save`(scale): 60/11 against 71/11 under condition A, so either the 1.0f web
reaches `totalsave` 72 at its present eleven components -- twelve more weighted
references, or two more at loop depth 1 -- or the scale carrier drops to
`nocs` 14 at its present total. One form measured this pass gets startlingly
close by accident: reading the case-0 scale use from its global directly, on top
of condition A, takes the scale carrier to `save` 5.545455 against the 1.0f
web's 5.454545, a gap of 0.09. It costs +16 bytes as written, but it shows the
two saves are reachable to each other from source.

A is the harder half, because the +8 bytes is a genuine spill pair and the
target has the same 368 instructions. The 1.0f web crosses the same call without
one because uopt rematerialises a constant; the scale carrier is a global load
and uopt spills it instead. A form that makes the scale carrier's live range
reach that call while still being rematerialised at each use is the missing
piece, and it is worth noting that the carrier is already rematerialised
everywhere else -- it has ten live-range components against eight references.

#### 2026-09-11, lane p7-fp: the forbidding rule is named, and the cost of condition A drops from a spill pair to one store

Baseline reproduces at 1472 bytes, 368 of 368 instructions, delta 0, 48 masked,
and the instrumented toolchain's text section is byte-identical to the tree's
object, which is the identity gate. `CDX_FORCE=p1:w35=c28` reproduces at 9
masked, delta 0, scored on the forced object directly. Two new facts settle
what the previous pass could only bound.

**The colour table is not what this shard has been recording, and the two
lowest entries were wrong.** Decoded by forcing the case-1 fraction web -- three
references, one component, so its move is a clean three-mention swap in the
float histogram -- onto every colour in turn: c24 f0, c25 f2, c26 f12, c27 f14,
c28 f16, c29 f18, c30 f20, c31 f22. Each force moves exactly three mentions off
the register the web held and onto the named one. **So f4, f6, f8 and f10 are
the registers outside the colour table, not f0 and f2**, and the section above
that records c24 f8 and c25 f10 is wrong on those two rows. Everything it
concluded from c26 upward survives, because c26 through c31 decode identically.
Two consequences worth keeping. The case-1 fraction web is on f0 and the case-0
scale-target web is on f2, not f8 and f10. And the note in the overlay 79 shard
that reads a forbidden mask of 0xf0 as f0, f2, f12, f14 is correct, not a
misreading; it and this shard disagreed and the overlay 79 note was right.

The bit encoding in the decision records is bit equals 31 minus colour,
confirmed on three independent rows: the carrier's own available mask, the 1.0f
web's forbidden mask under a force, and an overlay 79 web whose three declined
forces land exactly on its three forbidden bits.

**The float histogram proves the target's allocation exactly, and it proves the
target does not spill.** Candidate against target, float registers only: f0 4/4,
f2 3/3, f4 13/14, f6 12/14, f8 12/14, f10 12/14, f12 9/1, f14 9/9, f16 12/8,
f18 10/15. Under `p1:w35=c28` every one of the ten becomes identical. So the
target is this candidate with one colour changed, f12 carries nothing but the
outgoing argument, and no float web anywhere is on f12. Separately, the
candidate's whole opcode histogram already equals the target's, including zero
float traffic to the stack on either side, so **the target has no float spill
and uses no callee-saved float register at all.** Any adopted form must keep
both.

**The forbidding rule, stated and controlled.** A web is forbidden a call's
float argument colours exactly when it is live in that call's own basic block
at or before the call. Three controls, each at delta 0 or with the size change
accounted:

- Giving the call a second float argument adds c27 to the 1.0f web's forbidden
  mask, on top of the c26 it already had. So the forbidden colours are the
  argument registers of that call, not caller-saved registers in general.
- Putting a single float store of the loop-carried carrier in case 0 before the
  call takes the carrier's forbidden mask from c24,c25 to c24,c25,c26 -- and
  **emits no spill**, because the value dies at the store rather than crossing
  the call. The stack traffic census is identical to the target's. The whole
  cost is the store itself, four bytes.
- The same store with the call's float argument replaced by an integer one
  leaves the mask at c24,c25. The same store moved into case 2, a block with no
  call, also leaves it at c24,c25 while still adding ten to the carrier's
  totalsave. So the forbid needs the call, and it needs the reference to be in
  the call's own block before the call.

This replaces the previous pass's condition A. Condition A is not "live across
the call" and does not cost a spill pair. It is "referenced in the call's block
before the call", and it costs exactly one instruction.

**Which sharpens the blocker to one sentence: the pre-call reference has to
emit an instruction.** Eight forms that would have made it free were measured,
each byte-identical to the base and, more to the point, each leaving every
decision record unchanged -- the carrier stays at totalsave 71 over nocs 10 with
forbidden c24,c25:

- a void cast of the carrier, of the carrier times one, and of the carrier plus
  zero, all three placed in case 0 immediately before the call;
- a self-assignment;
- a comma expression discarding it;
- a reference inside `if (0)`;
- a conditional expression with the carrier on both arms;
- an assignment into an otherwise unused float local. This one is not
  byte-identical -- it scores 59 at delta 0 because the unused float still takes
  a frame home, which is L99 -- but the carrier's records are untouched, so it
  is the same negative.

A dead pre-call definition of the carrier, immediately overwritten after the
call, is also byte-identical with unchanged records: uopt eliminates it before
the web builder runs.

**The L109 probe family is closed on this function, checked in the records
rather than in the score.** All three reliable identity probes -- or with zero,
and with minus one, exclusive-or with zero -- plus five float forms, placed at
loop depth one, are text-identical to the base AND leave every `p1dec` record
identical. No new web is created and no totalsave moves. This is the check the
coordinator asked for before building on a probe, and it comes back negative:
whatever L109 reaches, it does not reach this procedure.

**So the reopen condition, restated.** A reference to the scale carrier inside
case 0, positioned before the float-argument call, that costs no instruction.
That reference gives condition A at four bytes with no spill; condition B still
needs the 1.0f web's save above the carrier's, which under the store form is 60
over 11 against 81 over 11.

One reframing the corrected table offers, which is worth checking on a function
that has it: **c26 and c27 are f12 and f14, which are the first and second float
argument registers, and the argument rule forbids both at once.** Giving the
call a second float argument forbids c26 and c27 together to any web live at it,
which would supply conditions A and B in a single edit and make the 1.0f web's
ratio irrelevant. It does not apply here -- this call's second parameter is an
integer, and changing that costs a conversion -- but on a sibling whose target
call already takes two floats, the whole two-condition problem collapses to the
one-instruction pre-call reference. Measured as a diagnostic rather than
inferred: the pre-call store together with a second float parameter on that call
takes the carrier's forbidden mask to c24, c25, c26, c27 and the carrier lands
on c28 f16, the target's colour, with the 1.0f web's ratio playing no part. The
form is not adoptable here because the second argument is fiction and the 1.0f
web is pushed to c29, but it proves the rule supplies both conditions at once.

Two arithmetic dead ends closed while checking that. Adding pre-call references
cannot buy condition B: each adds ten to the carrier's totalsave and one to its
nocs, and 81 plus 10k over 11 plus k rises, so more references always move the
carrier further above the 1.0f web, never below it. And the carrier cannot be
pushed below the 1.0f web by component count either -- at totalsave 81 it needs
nocs 15 to fall under 5.4545, against the 11 it reaches.

Two further negatives from this pass. Qualifying the four float globals `const`
changes nothing anywhere, so IDO 5.3 does not use const to let a global load be
rematerialised across a call, and that route to a spill-free crossing is shut.
**And the second intrinsic forbid in this procedure turns out to be L101 in the
float bank.** The case-0 scale-target web arrives with c24 forbidden before any
float web is coloured, its cost list starts at c25 rather than c24, and a direct
force onto c24 is declined -- so it is a real forbid and not a post-hoc mask.
With the corrected table c24 is f0, which is the float return register, and that
web's value is multiplied by the call's own result, so its range reaches the
call result. Removing the call from case 0 removes the forbid. That is exactly
L101 -- a web whose span reaches a call result is not offered the return
register -- holding for f0 as it does for v0. The case-1 fraction web, which is
in a block with no call, has an empty forbidden mask and takes f0 freely.

So this procedure has three forbidding rules and all three are now named: a
colour taken by an interfering web that decided earlier; the float argument
registers of a call the web is live at, at or before the call; and the float
return register of a call whose result the web's range reaches.

**Cross-function note, corrected.** Overlay 79 is not the same problem. Its own
float ladder was decoded this pass and is reported in its shard.

**A stale premise, for the record.** The dispatch that opened this lane stated
the requirement as two further float webs above save 7.1, which the previous
section of this shard had already corrected to the two-condition form. The
two-web reading is refuted independently by the float histogram: any such web
would sit on f12, and the target names f12 once.


#### 2026-09-12, lane p19-reopen: real call ABI limits the arity lever

Assignment was base-only. Baseline and retained C are 368 words, delta zero,
frame 0x60, first +0x94, 49 raw and 48 masked differences. Buckets are
320 exact / 46 naming / 0 immediate / 2 really different before and after.
Frame and address-window censuses reproduce the previous fp-colour diagnosis.
The configured instrument passes full-TU text identity; ordinal 1 emits p1.
The inherited accepted w35-to-c28 force is already causal evidence and was
not re-swept or mistaken for a source match.

The dispatch's two-float shortcut does not describe this call's actual ABI.
`include/overlays/overlay_027.h` declares `func_8002A878(f32, s32)`, and the
existing p7-fp proof explicitly measures the cost and fiction of changing the
second parameter. L142 applies to the argument colours actually loaded; it
does not authorize adding a second float argument. Both the pre-call emitted
reference and the remaining f14 competition must be satisfied by real source.

Four new controls test the remaining source territory. Reusing scaleFactor
for the call result before restoring the scale gives 58 masked at delta zero,
buckets 316/43/0/11; the persistent carrier remains total 71 over 10 and c26.
Assigning scaleFactor to 1.0 before using it in the subtraction gives 57
masked, buckets 311/55/0/2. This creates a separate fraction web (57, total
20 over one component, c26), while the persistent scale takes c27 and the
constant c28. Thus a blanket claim that every reuse necessarily coalesces is
too broad, but this real new web occupies f12, contradicting the target's
outgoing-argument-only f12 use. It is not the wanted solution.

L144 address-form call-input reuse gives 333 masked and adds 24 bytes;
address-form scale reads give 333 and add 20 bytes. The latter removes the
coloured scale carrier at the cost of stack traffic absent from the target.
The four controls provide no better retained residual. The measured call ABI,
no-spill requirement and emitted-reference obstacle survive; there is no
basis for another identity-op or fabricated-arity sweep. A next source
hypothesis must consume a pre-call reference in an already required operation
and preserve the actual argument setup, then show c28 in accepted records.

Commands: alignment, residual map, register/frame censuses, configured
stock/instrumented compilation and direct object comparison,
`tools/finalize_plateau.py`, `tools/gates.sh`. Attempts, objects and records
remain in private external scratch. The 22 relocation records are not a
promotion proof: the inherited audit does not establish all offset/type and
identity pairs. C remains guarded; the unchanged assembly fallback is what
passes full-ROM verification. No matching credit or body edit is adopted.

#### 2026-09-12, lane p23-lastmile: complete single-colour landscape and winner lattice

Baseline reproduces 49 raw and 48 masked differences over 368 words at zero
size delta: 320 exact, 46 naming, and two structural rows, first +0x94.
The stock and instrumented full-TU text agree. The Ucode-entry receipt maps
this function to procedure 1, with 51 decisions in a six-procedure TU.

The exhaustive same-kind landscape covers every one of its 23 coloured webs:
195 forces compiled, 140 accepted at zero size delta, 52 changed size, and
three failed strict acceptance receipts. Eight forces beat the baseline:

- p1:w35=c28 gives 9; p1:w35=c27 gives 25.
- p1:w64=c28 gives 19; p1:w64=c27 gives 28.
- p1:w120=c28 gives 19; p1:w120=c26 and p1:w120=c27 each give 28.
- p1:w257=c28 gives 25.

These are the persistent scale, case-0 scale-target value, case-1 fraction,
and 1.0f constant respectively. Every winner is floating-point; no integer
force wins. All eight radii overlap. The 64/120 c28 radii are subsets of the
scale-force radius, so they are competing routes through the same bank, not
independent savings.

The stock/instrumented-fidelity-gated force_lattice run tested all subsets of
the four best single forces. Expanding to every distinct-web combination of
all eight winning choices gives 71 nonempty points: 31 accepted at zero size
delta, 40 refused, none accepted with a changed size. The minimum among these
accepted points is nine, from w35=c28 alone; next is twelve with w120=c26
added. The refused points remain unscored. This is a measured floor for this
nominated choice set, not for arbitrary forces or changed source. In the
best-choice lattice, w64=c28 plus w120=c28 scores 22, an antagonistic pair;
several other pairs cannot apply both requested colours at all.

Aligned comparison of w35=c28 against the unforced object removes 39 naming
rows across seven windows and introduces none. Its retained residual is seven
naming and two structural rows. Thus its positional gain hides no local
regression or insertion shadow. The existing source requirement is confirmed:
the scale carrier must reach c28 while preserving the real float/integer call
ABI, no float spill, and the current executable size. Recolouring either
short-lived fraction instead reaches a worse floor; it does not create an
independent source task. The R4300 scheduler flag that closed overlay 101 is
byte-inert here and is not adopted.

Three final carrier controls all regress, with stock/instrumented text
identity checked for each. Carrying the default arm's narrowed intensity in
the existing tail union gives 214 masked and adds 28 bytes; the scale remains
at total 71 over ten components and c26. Carrying the scale through a float
member of that union gives 333 and adds 28 bytes, losing the coloured carrier
in favour of memory traffic. Reusing fraction for the scale after the
interpolation gives 59 at zero size delta: total 81 over ten components,
still c26, with the same forbidden colours. Original float declarations
were retained as diagnostic frame controls; no such edit is adopted.

Stall evidence: the last useful result was the complete force landscape and
its measured combination floor. The three carrier attempts add no better
residual, target identity, or new route past the already-recorded argument
forbid/no-spill obstacle. They reproduce the existing carrier, memory, or
save-ratio failure classes. Restore the original 48-word candidate and stop;
a next source attempt must identify an already-required pre-call operation
that supplies the missing live reference under the actual ABI, rather than
repeat identity probes, fictional call arity, or union carrier spellings.

Private evidence is preserved under build/p23/o027/: baseline and Ucode,
procedure receipt, complete colour landscape, copied force_lattice receipts,
all 71 option-lattice results, aligned best-force comparison, and each source
control's source, object, score, first mismatch, and allocator records.
Commands include residual_map, allocator_trace_receipt, web_footprint,
force_lattice and its run_cell helper, configured compiler controls,
finalize_plateau, and tools/gates.sh. C remains guarded; the 22 relocation
records are counted but do not establish a new complete identity proof.
No executable matching credit is claimed for this function.

#### 2026-09-12, lane p24-second-mid: the required call input does not retain a scale-web reference

The base-only gate passes. Configured baseline reproduces 368 words, zero
size delta, frame 0x60, 49 raw and 48 masked differences, first +0x94.
Aligned rows are 320 exact, 46 naming and two structural, with no insertion
shadow. The existing exhaustive colour landscape and nine-word diagnostic
floor were read and not repeated.

This pass tests the named next operation directly: the already-required
floating call input. Assigning that input to scaleFactor and passing the
carrier into the unchanged float/integer call is full-TU text-identical to
the baseline. Thus the source occurrence disappears before it can supply the
missing live reference; this is not a refused-force result. Reusing the
carrier for the returned fraction through the required scaleTarget update,
then restoring its scale value, gives 58 masked at unchanged size. Combining
input and fraction reuse gives the same 58. Both fraction forms remove three
naming rows near the call but add five paired structural rows and two
candidate-only/target-only pairs; the large downstream naming region is
unchanged. Aligned comparisons expose the regression directly.

All three forms preserve the actual call argument types and call order. The
input control is byte-inert and the other two reproduce the already-recorded
carrier/schedule failure class. No available route through these required
operations supplies the pre-call reference at zero cost, so stop on that
mechanism evidence. The baseline is retained; a future attempt must identify
a different operation whose scale reference survives compiler substitution
without a float spill. No candidate body or ABI change is adopted.

An external-path baseline control is full-TU text-identical before using that
private compile path. All sources, configured objects, scores, first mismatches
and aligned per-window comparisons remain under
build/p24/func_overlay_027_F0000064_187BA3C/. The configured candidate has
22 text relocation sites; no new complete runtime identity proof is claimed.
Commands: lane_status, configured compilation, residual_map --object/--against,
full-TU byte comparison, finalize_plateau and tools/gates.sh.

#### 2026-09-26, lane p65-o027: shortening the scale live range does not leave f12

Restored baseline scores 1472 bytes, delta 0, frame 0x60, 49 raw and 48 masked
words, first mismatch +0x94, 22 text relocations. The three retained closures
stay: updateRate's copy is still the first local, post-loop fade stays cached
in value, and the case-3 intensity read stays in phase. No colour or force
sweep.

Hypothesis: shorten or split the scale value's live range, by a source
spelling, so it is not the register a call keeps for an argument. Three
spellings, each reverted because size grew (a delta-0 masked drop was the keep
rule):

- Fresh function-scope float, assigned from gO27Scale8 after the ease call,
  used for the case-0 scale multiply, then copied into scaleFactor. Size
  delta +4, 272 masked, first mismatch +0x0. The new local takes a frame home.
- Recompute at each use from the existing tail union's float member, replacing
  the scaleFactor local. Size delta +28, 333 masked, first +0x8C. Same
  memory-traffic result already recorded for a union scale carrier.
- Block-scoped float with that same post-call lifetime. Size delta +4, 272
  masked, first +0x0. Block scope does not avoid the frame home.

None stayed at delta 0, so none could reduce the 48 masked words. The original
body is restored. Stall: three consecutive spellings produced no better
residual, no new target identity, and no zero-cost split of the scale carrier.
The open residual is still the long-lived scale in f12 rather than f16, plus
the two instruction-order pairs.

Commands: tools/score_symbol.py on the restored body and on each spelling,
stock tools/ido/cc, tools/finalize_plateau.py.

<!-- plateau-handoff:func_overlay_027_F0000064_187BA3C:end -->
