<!-- plateau-handoff:overlay14CreateValue:start -->
### `overlay14CreateValue` plateau handoff

- source: `src/overlays/o014/overlay14CreateValue.c`
- score: 13/96 words
- frame: 0x28
- relocations: 15
- first mismatch: +0x54
- summary: Fresh 10-draw baseline confirms the extra +0x24 pointer home; declaration-order probe moved no draw or emission line and was restored.
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

<!-- plateau-handoff:overlay14CreateValue:end -->
