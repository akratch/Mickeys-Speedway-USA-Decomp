<!-- plateau-handoff:overlay14CreateValue:start -->
### `overlay14CreateValue` plateau handoff

- source: `src/overlays/o014/overlay14CreateValue.c`
- score: 2/96 words
- frame: 0x28
- relocations: 15
- first mismatch: +0x158
- summary: tail count load versus key store; a declared carrier sequences the load first but colours it a1; named-array key stores stamp count-versus-chosen-LDA at an extra la; comma-in-plus folds.
- base: `3169297845d9e4b3843c03be16cfe6d51358d280`
- boundary: overlay 14 `+0x6FC..+0x87C`, 384 bytes / 96 words, no
  padding or export; two local callers at `+0x3C0` and `+0x40C`
- configured result: exact geometry and frame `0x28`, 83/96 masked
  relocation-aware positional words (14 raw differences), first workbench
  mismatch `+0x54`

Fresh lane measurement (2026-09-11, lane/p7-mix): a zero-byte `if (1)` region
around the chosen/call block improves the configured candidate from 14 masked
differences to 13 and exposes one coherent `v1 -> t1 -> t2 -> v1` ring. The
equivalent `do { } while (0)` spelling and register-qualified chosen pointer
are flat. The target still uses four homes versus the candidate's five, with
the extra candidate home at `+0x24`; no exact result was promoted.
- scheduler evidence: a native IDO 5.3 `as1 -R` capture was trace-off
  identical in `.text`, `.data`, `.rodata`, and relocation tuples. The first
  block contains independent slot-base and end-pointer HI16/LO16 chains. Their
  low halves are simultaneously ready and the earlier logical source line wins.
- strict gain: keeping `slot = gOverlay14Slots28; scan_loop:` on one physical
  line preserves C tokens and reverses the tied low-half choice, closing
  `+0x18/+0x1C` and improving the reproduced plateau from 80/96 to 82/96
- relocation proof: fresh configured full-TU preflight is complete. Target and
  candidate each carry 15 records; all 15 offsets/types and stable/effective
  runtime identities align, with zero unresolved identities. The selector at
  `+0xAC` is the existing `s32 frontGetLanguage(void)`: shipped runtime identity
  `resident:+0x39DFC` plus the resident base selects its existing linked symbol
  at `0x8003A24C`. Its 20 linked bytes equal the original ROM range. Replacing
  the unauthenticated extern/call spelling preserves physical source lines and
  leaves candidate text bytes, frame, geometry, and positional score unchanged.
  No alias, symbol policy, or post-compile instruction edit was introduced.
- diagnostic limits: workbench's synthetic object comparison still reports
  import-fix with raw symbol-label differences; the full preflight authenticates
  runtime identities separately. Geometry edit distance is 12, opcode distance
  is 2, and the owned extent delta is zero. This is not an exact C result or a
  linked-trial recommendation; the NON_MATCHING fallback remains in place.
- residual: the `+0x54` workbench row is the already-authenticated active-slot
  data addend in an unlinked candidate. The first source/codegen residual is
  the chosen-pointer web around `+0xA4`: target uses pool register `v1` and one
  `sp+0x18` home, while the candidate introduces `t3` and later spills the
  copied `slot` web at `sp+0x24`. The final value/count block also carries a
  two-word temporary-ring shift.
- exhausted controls: the prior 119-configuration flag lattice remains closed.
  A call/slot/switch physical-line tie was byte-flat. Direct one-pointer and
  nonvolatile-pointer forms grew the owner by four bytes. Assigning `slot`
  before the selector regressed to 11/96 and displaced nine relocation sites;
  an active-pointer carrier regressed to 76/96, and a scan-key carrier to
  74/96.
- bounded permutation: one ten-minute, two-thread, relocation-annotated batch
  improved its scratch score from 345 to 210 but found no zero. Its best forms
  used forbidden inert labels/guards or changed case semantics; the natural
  carrier results regressed in the real TU and were rejected.
- next lever: retain this source. The operational selector-identity correction
  does not reopen manual source permutation: 14 differing words exceed the
  ADR 0016 manual gate. Await the reviewed transaction-tool integration and an
  explicitly bounded standing batch; a new manual attempt still requires its
  own admissible evidence. Do not repeat flags, physical-line ties,
  pointer-direct, assignment-order, or carrier forms.

#### 2026-09-17, lane lm-o014: the spill is the alias poison, not the three bases

Baseline reproduces: 2 masked (3 raw), delta 0, frame 0x28, 15 relocations,
first +0x158, aligner 94/0/0/2. Identity-gated instrumented cc against the
tree object (byte-identical .text). Nothing adopted; best remains 2.

The two structural rows are a swap of the count load and the key store.
ugen already emits the count address before the key store and the count
load after it; as1 hoists the load over the volatile value reload but not
over the key store. Listing replay through as0/as1 is identity-clean for
this TU. Inserting a noalias stamp between the slot register and the count
address (either operand order, at the la of the count or at the count
load) produces the target tail. Swapping ugen's emission so the count load
precedes the key store does the same. A noalias versus sp on the slot
register does not. Volatile around the key store does not.

The instrumented alias profile names the query: one may-alias between the
slot (isvar) and the count address (islda). A no-call mini of the tail
shape queries no-alias (islda versus islda) and ugen stamps the
count-versus-slot fact. The same mini with one call queries may-alias
(isvar versus islda) and the stamp is gone -- the spill retags the
register, even when the pointer has a single named base. onebase_slot,
onebase without the or-with-zero, and drop_or all still query may-alias.
The three-base diagnosis is therefore the wrong variable; the spill is.

A post-call assignment `slot = gOverlay14ChosenSlots28` retags the existing
slot register islda, stamps the wanted count-versus-slot fact, and scores
13 at plus four (the extra la, and it writes the array base). Identity
recasts that fold (or with base-minus-base, xor with base-xor-base, a temp
initialised from the named array then copied from slot) stay at 2 with no
stamp. Recovering the index as `slot - base` is 19 at plus 20.

Sequencing the count load in C without that stamp: increment-before-key is
4 at delta 0 (right registers, the add and count store stay glued to the
load). Reading the count into `kind` is 3 at delta 0 (right schedule, the
value sits in a1 instead of a ring temp -- L145: a declared local is never
a ring temp). A fresh or block-scope local is 5 to 21. Discarded L109
probes, comma loads, and `*(s32 *)&count++` are inert at 2.

On the retained shape both `index = 0` and the or-with-zero def are still
load-bearing (91 at plus 20, and 92 at minus eight). A discarded
or-with-zero use does not lift the save. Separate scan/free cursors are 57
at plus four; merging all three names is 81 at minus eight.

Next: a post-call islda retag of the spilled slot register that does not
emit an extra instruction (or that replaces the stack reload one-for-one
while keeping the chosen address), or a ugen-temp count load sequenced
before the key store -- the `kind` carrier's schedule without a symbol
web. Do not repeat three-base splits, L109 discarded probes, increment-
before as a source order, or declared count carriers.

#### 2026-09-17, lane w2-o014: a1 is the unique declared-carrier colour; comma-in-plus folds

Baseline reproduces: 2 masked (3 raw), delta 0, frame 0x28, 15 relocations,
first +0x158, aligner 94/0/0/2. Identity-gated instrumented cc against the
tree object (byte-identical .text). Nothing adopted; best remains 2.

The `kind` carrier is confirmed at the object: it emits the ROM schedule
(count load, key store, value reload, add, count store) and differs in
three naming words -- the load sits in a1, so the add/store use the first
ring temp rather than the second. At that tail, v0 holds the count
address, v1 the spilled slot, a0 the value and a2 the key, so a1 is the
first free caller-saved colour. `index` as the carrier is 5 (it wants v0,
which is the count address). `alternate` is 55 at plus four. A fresh or
block-scope local is in the same family: any symbol web at that site
takes a1, never a ring temp (L145/L130).

Named-array key stores through `gOverlay14ChosenSlots28 + (slot - base)`
stamp `.noalias` between the count address and the chosen-array LDA, at
plus twelve and 17 masked. The extra instruction is the `la` of that
array; the stamp names the array-base register, not the spilled slot.
Keep-index retags and `gOverlay14ChosenSlots28[index]` rematerialise
(plus 28, 93-94 masked). `index = 0` remains load-bearing.

`count = count + (slot->key = key, value = slot->value, 1)` and the
`+=` spelling fold to the baseline: cfe evaluates the side-effecting
comma first, then the load. Volatile on the left-hand load still emits
the key store first (3). Identity recasts (xor with base-xor-base,
addr-of-zero plus delta) emit the extra `la` or fold with no stamp.

Next: an islda/isilda retag of the spilled slot that copy-props onto the
existing stack reload, or a ugen-temp live across the key store that is
not a declared local and not a comma operand of `+`. Do not repeat
declared count carriers, comma-in-plus, named-array key stores that
recover the index as `slot - base`, or keep-index rematerialisation.

#### Re-open under laws L90 / L94 (2026-09-10, lane/c3-reopen2)

- lever set the closure actually searched: the 119-row flag lattice, a
  call/slot/switch physical-line tie, direct one-pointer and non-volatile
  pointer forms, assignment order, three carrier forms, and one bounded
  ten-minute permutation batch. All of them are spellings of the pointer
  carriers; none of them changed which *object* the slot reads belong to.
- **L94's mechanism is present here, and it confirms the closure rather than
  reopening it.** The target object names three separate address
  materialisations at one and the same overlay data address, so the natural
  reading is that the three slot externs are one C array. They are not.
  Merging the two that share a region makes uopt open a single address web and
  lose 4 bytes: the entry array merged with the free-scan array is -4 bytes
  and 95 masked words, the entry array merged with the chosen array is
  -4 bytes and 94, and merging all three is -8 bytes and 80. The one pair that
  is *not* co-region -- the free-scan and chosen bases, separated by the
  bounded do-while -- merges at delta 0, and still scores 17 against the base
  14. So L94's co-region trigger is exactly what proves the three separate
  externs are required by geometry: any co-region merge is paid for in bytes
  the target does not spend.
- **L90 has nothing to reach.** Both loops are already byte-exact. The scan
  loop lies inside the exact prefix, and the bounded counter test that follows
  it matches instruction for instruction. There is no exit test left to
  rewrite.
- the deciding variable, named: a frame-cell one. The target spills one pointer
  web at one home and reloads it around all three calls. The candidate carries
  the volatile chosen pointer at that same home *and* spills a second copied
  web three cells higher, which is the whole of the constant class in the
  residual and the extra block-local temporary that follows it. A uopt cell
  census confirms the candidate declares six memory cells for five source
  locals.
- what was tried against that variable and failed: every form that removes a
  pointer costs +4 bytes -- the one-pointer form scores 76, the non-volatile
  carrier 76, dropping the copy and using the volatile carrier directly 69,
  and making the scan pointer volatile 104. Moving the carrier's declaration
  to the front or the back of the block regresses to 19 and 23.
- **verdict: the closure was correct and remains correct under L90 and L94.**
  Resume only with a form that removes one *cell* without removing a pointer.

#### 2026-09-12, lane `p12-tight`: L145 and L144 both reach this function and neither removes the cell

Baseline reproduces: 384 bytes, 13 relocation-masked words (14 raw), size delta
0, frame 0x28, first mismatch +0x54. Aligner: 83 byte-exact, 5 register naming,
4 immediate only, 4 really different. No edit adopted.

The standing reopen condition is "a form that removes one cell without removing
a pointer". Two laws postdating this shard were tested against it.

**L145 -- delete the carrier -- is refuted here, and the refutation is clean.**
Writing every use in the chosen block as the global's own subscript, with
neither the volatile carrier nor the copied scan pointer declared at all, costs
4 bytes and scores 75, in both the region-opened and bare spellings. L145's
boundary condition is met (the expression is spelled identically on both sides
of every call) and the mechanism still does not fire, because the target does
not want the value in a ring temporary: its object spills a pointer web to a
home and reloads it around all three calls. A function whose target SPILLS is
outside L145's territory.

**L144's address form exactly reproduces the volatile carrier here, which is a
useful negative control.** Declaring the chosen pointer as a plain local and
reading it through its own address at every use, with the copy into the scan
pointer kept, is BYTE-IDENTICAL to the retained volatile form: 13 words, delta
0, same four homes plus the extra one. So on this function `volatile`'s two
effects are not separable in the way L144 describes -- the scheduling edges buy
nothing, and dropping them costs nothing. Taking the copy away from either form
is 64 at delta 0 (address-read spelling) or 71 to 75 at +4 bytes (direct
spellings), so the copy is load-bearing in every spelling of the carrier.

Six forms measured this pass, none below 13. The reopen condition is unchanged
and the two cheapest laws that looked like they reached it are now spent.

#### 2026-09-16, lane nx-a: the target's pointer is spilled to its own home, and the ratio that blocks it is named

Baseline reproduces 384 bytes, delta zero, 13 masked and 14 raw, frame 0x28,
first mismatch +0x54; aligner 83 exact, 5 naming, 4 immediate, 4 structural.
Nothing adopted; eight cycles.

Read off the objects: the target computes index shifted by three as a ring
draw (t1) and sums it straight into v1, which is one pointer web spilled to
its own home 0x18 around all three calls; the candidate colours the shift as
a web (v1), writes the sum to the volatile chosen home, and copies it into the
register slot web whose spill goes to a temp at 0x24. The tail's count load
is above the key store in the target and below it here.

Measured: a plain (non-volatile) chosen is copy-propagated and rematerialised
after each call from a spilled index (frame 0x38, 75, plus four); that holds
with the loop walked by slot increments (93), with every index spelling (u32,
u8 pointer, plus, shift), with declaration order, and with index reused as the
switch variable (22 base, 67 one pointer). Killing index after the pointer is
formed (a dead `index = 0`, or `index++`) is the one thing that makes uopt
spill the pointer to its own home and draw the shift from the ring: 54 at plus
four, and the four bytes are the arm merge, because the pointer takes a3
while the constant 1 holds v1.

With one `slot` variable through the scan loop, the free loop and the calls
(no chosen), the shape from +0x9C to the tail is the target's except the
frame, and the census names the last decision: slot (web 0) is 57 over 7 and
value (web 3) is 25 over 3, so value is decided first, takes v1, and slot
falls to a2; the base's copy form has slot at 57 over 6 because the copy's
def sits after the call. Two probes on slot raise both terms (59 over 8) and
do not flip it. Testing `slot->value` instead of `value` in the tail drops
value to 24 over 3 and flips the order: slot v1, value a0, every colour the
target's, at 31 words and plus four for the reload the spelling adds. Taking
value out of the scan loop also flips it (38, delta zero) but breaks the
loop's a0. Per-arm value definitions add a reference (26 over 3). A non
register slot still spills to a temp (frame 0x20); two unused pointer locals
do not enlarge the frame here.

Next hypothesis, in order: find a tail spelling that removes exactly one
weight-one reference to value without adding a load (the volatile struct
field forces a load per read, so the candidate is the return path); and a
declaration that gives the single slot web a home at 0x18 with the frame at
0x28, which a separate declared chosen achieved (the idx0 form's frame is the
target's) but the one-web form does not. The lane report is
docs/lastmile-phantom-web.md.

#### 2026-09-16, lane w1-b: 13 -> 2 on the frame home and a PRE split, six cycles

Baseline reproduced at 13 masked (14 raw), delta 0, frame 0x28, aligner
83/5/4/4. Retained: 2 masked (3 raw), delta 0, frame 0x28, first +0x158,
aligner 94/0/0/2; the two rows are the count load the ROM schedules above
the key store. Every score is `tools/score_symbol.py` or the same comparator
on the configured TU; nothing forced.

What the residual was, read off the objects and the records: the target's
pointer is one symbol web (scan cursor, free-loop cursor and chosen slot)
coloured v1 and spilled around all three calls to its OWN home at 0x18, its
shift is a ring draw, and the tail reloads slot before key. Ours had the
pointer flowing through a volatile home plus a `register` copy spilled to a
temp at 0x24, and a PRE'd shift web.

- Cycle 1 (address-taken and separated-cursor forms): taking `&slot` in a
  dead branch removes the pointer from colouring altogether (its symbol web
  vanishes from the ladder), 61 to 94; the L55 mechanism is the wrong one
  here. A separate scan cursor confines the cursor web to the loop, where
  it takes v0 and pushes `value` to v1 (75, plus four).
- Cycle 2 (plain `chosen` plus index kills and pointer probes, nx-a's
  cycle-5 shape): every cell 58 at plus four with the FIRST difference at
  +0x4, so that shape is not "the target from +0x9C on" as the previous
  entry says; the head moves because the cursor web no longer spans the
  index blocks. Probes on the pointer are inert there.
- Cycle 3: one `slot` symbol through everything, `index = 0` after the
  pointer (so uopt cannot rematerialise it from a spilled index), and ONE
  or-with-zero read of the pointer: 94 at minus eight to 25 at delta 0. The
  read lifts the pointer web's net from 57 to 59 over nocs 7, 8.43 against
  `value`'s 8.33, so the pointer is decided first and takes v1, `value` a0,
  the slots-end address v0. A second or third read is inert; unused f32,
  f64 and array locals do not enlarge the frame (L99's wording is wrong
  about f32 here).
- Cycle 4 (frame): declaring `slot` FOURTH (`value, index, kind, slot`)
  moves its reserved home to 0x18 and the frame to 0x28: 25 to 10. A
  coloured pointer spilled around calls spills to its own reserved home,
  and the homes descend in declaration order. Declaring `kind` first is
  19; unused pointer locals, a volatile pad and a dead `chosen` copy are
  all 25.
- Cycle 5 (PRE): `slot = gOverlay14ChosenSlots28 + index` (or an explicit
  `(u32)index << 3`, or a `u8 *` spelling of the loop's subscript) is a
  different IR name from the free loop's `&gOverlay14FreeSlots28[index]`,
  so the shift is no longer PRE'd into a two-block web coloured a0 and is
  drawn from the ring as `t1`, realigning every later draw: 10 to 2.
  `index * 8` stays PRE'd (10). Computing the pointer before the bound
  check is 20.
- Cycle 6 (tail order): reading the count into a local first is 21,
  incrementing before the key store 4, between the stores 5. The ROM's
  order is not a source order; it is as1 hoisting the count load over the
  key store, which needs a `.noalias` between the pointer and the count's
  address. ugen stamps that only for a pointer whose provenance is one base;
  our pointer symbol has three (scan, free, chosen). On the two-symbol
  shape the chosen pointer has bestcost 4.0 against save 1.2 and is still
  coloured (a3) rather than split, so the target's own-home pieces are not
  a split either.

Next: a pointer with one base for the tail without a second symbol web
that steals v1 or lands on a3, or an emission-order spelling of the tail
that as1 keeps; and check whether the `index = 0` kill and the or-with-zero
read are both still load-bearing on the retained shape (they were adopted
in sequence and not re-tested individually after the declaration move).
<!-- plateau-handoff:overlay14CreateValue:end -->
