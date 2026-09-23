<!-- plateau-handoff:overlay89UpdateStateAndParticles:start -->
### `overlay89UpdateStateAndParticles` plateau handoff

- source: `src/overlays/o089/overlay89UpdateStateAndParticles.c`
- score: 136/136 words
- frame: 0x88
- relocations: 14
- first mismatch: none
- summary: ROM-exact and promoted; if(count--) guard around a do-while, value-returning spawn, secondaryHandle declared after particle, and two empty do-while statements that lift the callee toll so the particle-address web splits.
#### 2026-09-13, lane k1: authenticated draw-census follow-up

Fresh configured stock compilation reproduces 544 target bytes,
size delta 4, 98 raw and
98 masked differences, first +0x0.
Candidate frame is 0x90; the procedure-0 census records
29 draws and 230 emission records. Stock and traced full-TU text
compare identically. Static relocation counts are 14 candidate and
12 extracted target, with 0 identical offset/type/symbol tuples.
These are static measurements; overlay runtime identity proof remains separate.

Baseline paired buckets are 53 exact, 37 naming, 20 immediate and 22
structural, with five candidate-only and four target-only words. The candidate
has 137 words and frame 0x90; target geometry is 136 words and frame 0x88.

Three independent source hypotheses were measured after reading the source
and shard. First, replace the local volatile parameter with ordinary s32 and
spell both reads through its address. This tests L144's reload-versus-scheduler
edge distinction. The call and timer lines each lose two emission records,
while every draw and its sequence stay fixed. Full-TU text is byte-identical:
the parameter-home control does not reach the residual.

Second, move the z-coordinate initialization beside x and y, crossing only
nonescaping particle-local constant stores and preserving x/y/z load order.
The 29 draws and 230 emission records remain, but their joint GP/FP order
changes; each bank's own draw subsequence is unchanged. The positional score
improves to 97, but the paired aligned residual adds three naming and two
immediate rows. Four gap words disappear, leaving the total aligned residual
one row worse. This scalar improvement is not adopted.

Third, retain the original count into the second nonzero guard and decrement
inside that guard before the unchanged loop. The u8 count and outer nonzero
check prove identical iterations. Two count lines change emission attribution;
29 draws and their order are unchanged, and full-TU text is byte-identical to
baseline. It does not supply the target's count-transfer schedule.

All three source/object pairs have fresh named-Ucode mapping, stock/traced
full-TU fidelity and retained draw/aligned differentials. Stop early under
ADR 0018: these controls reject the identified parameter-home, coordinate-order
and guarded-count explanations, while the recorded aggregate/address/loop and
flag variants cover the remaining available forms. The frame, saved particle
address and post-call schedule remain unresolved. No colour sweep was repeated
and the original 98-word guarded baseline is restored.

The retained C remains guarded by its original assembly fallback. Sources, stock
and traced objects, frame and scalar measurements, draw profiles and aligned
maps remain ignored under build/k1/overlay89UpdateStateAndParticles.
Commands: lane_status.py, configured stock compilation, draw_census.py,
residual_map.py --object/--against where compared, finalize_plateau.py and
tools/gates.sh. No executable bytes are newly credited.

#### 2026-09-18, lane w21-o089: frame closed; extra s3 is type-1 particle address

Identity-gated instrumented IDO (stock and instrumented .text identical,
CDX_PROC=0). Confirmed baseline 544 B, size delta +4, masked 98, frame 0x90.
The extra callee-saved register is type-1 web 122 (particle address, nocs=3,
totalsave=10, coloured s1). Forcing p1:w122=s rematerializes the stack address
into a0, drops s3, and matches the target save area (f20 at 0x20, s0/s1/s2/ra).

Adopted source that closed the 8-byte frame surplus without a force:
s32 count instead of a homed u8 particleCount; randomScale declared first;
particle.z written with x and y (early-z on this new shape, not the rejected
late-shape form). Unforced result is 85 masked, frame 0x88 exact, size still
+4, 137 vs 136 words, relocations 14 vs 12, first +0x0. Aligned 67 exact,
39 naming, 11 immediate, 21 structural. Candidate-only words are the extra
s3 save and restore; one target-only word at +0xE8.

On that 85-score object, p1:w122=s scores 95 positional at size -8, but aligned
92 exact, 8 naming, 9 immediate, 28 structural, 23 vs 23 slots. Particle homes
match 0x50 through 0x80. The only remaining slot is secondaryHandle at +0x44
versus target +0x4C (8 bytes). Immediate rows are that displacement. Size -8
is the lost s3 save/restore plus the hoisted addiu collapsing into the loop
rematerialize, missing the target's count-copy or v1 copy.

Inert or worse on this shape (ADR 0018 after the last adoption): L97 around
spawn, loop, randomScale, or secondaryHandle; register f32; particle as
array of 1; first-member address and volatile-cast address spellings;
while(count--); deleting randomScale (loses f20, +12); deleting state and
primaryHandle locals (+32); extra declared s32 particleCount (frame 0x90,
94 masked); L144 parameter-home already retired. Colour lattice not run:
delta is still +4 and --every-colour refuses.

Next lever is a source form that splits web 122 (totalsave 10 vs s1 cost 9)
without growing the frame, then an 8-byte temp below the save area so
secondaryHandle lands at +0x4C, plus the missing count-copy into s0 and the
extra v1 copy of particleCount without a homed extra s32.

#### 2026-09-23, lane B2-ov2: matched and promoted

Cycle 0 reader: +4 bytes, one spill/reload pair (the extra s3 save and
restore, prologue and epilogue lines) and one target-only word, 71 aligned
after 14 shadow. The target-only word is the guard's dead post-decrement copy,
not a constant. Method: put a candidate shape in, force every coloured type-1
web to split with the instrumented uopt, and score the forced object directly.
That priced the shapes before any unforced work.

- `while (count--)` with `count` loaded straight into s0: +8. The guard copy
  comes back, but the load goes directly to s0 with no copy.
- Testing `state->particleCount` and assigning `count` inside the test, before
  the particle stores: the load is one CSE'd temp copied into s0, as in the
  target. Forced: delta 0 at 15.
- `if (count--) { randomScale = ...; do { } while (count--); }`: the
  randomScale load lands after the guard, as in the target, and the guard
  copy survives. Forced: 14.
- The remaining naming was two webs, the loaded temp and the post-decrement
  copy, taking v1 and v0 against the target's v0 and v1. Declaring
  overlay34Spawn as returning s32 fixes it. Forced: 9, with naming 0.
- Declaring secondaryHandle right after particle moves its home from 0x44 to
  0x4C, the target's. Forced: 3.
- Reading primaryHandle before secondaryHandle fixes the prologue schedule.
  Forced: 0.

The one force left was the particle-address web: totalsave 10, cost 9 at s1
(the L56 toll of 8 plus 1), so it is coloured. L56 prices the first
callee-saved register at nBB/4, and an empty `do { } while (0)` (or
`if (1) { }`) adds two blocks and no code; `if (0) { }` is folded and adds
nothing. Two of them lift the toll to 9, and the web splits at 10 <= 10.
Unforced result: 136 of 136 words byte-identical, frame 0x88, gmake verify OK,
check-overlay-syms up to date, promotion-proof PASS with 14 of 14 relocations.

This corrects the earlier handoffs on three points. The size gap was not the
frame and not the count copy. It was the loop shape plus an allocator toll
decision, and the toll is movable from source because it counts blocks. The
missing-copy and v0/v1 residue was an unprototyped callee's return type.

<!-- plateau-handoff:overlay89UpdateStateAndParticles:end -->
