<!-- plateau-handoff:overlay1TransitionState:start -->
### `overlay1TransitionState` plateau handoff

- source: `src/overlays/o001/overlay_001_tail.c`
- score: 0/237 words, promoted
- frame: 0x50
- relocations: 13
- first mismatch: none
- summary: Matched. One u8 phase local tested with a boolean not and then compared, and the route slot taken as an array element address indexed by the selector assignment.

Summary before this remeasure: The size-near pointer-update probe is byte-flat; promotion trial in=0/out=0 is a schedule-divergence build error, not equality. Retain the 160-word structural plateau.
- assignment base: `f8f3ec51a298dd0eddd0574a4313adbb1e39de9b`
- identity: Overlay 1 export-table row 1340 at `+0x3FD8`; the fallback proves the owned `0x3B4`-byte range through the next function at `+0x438C`, with no target padding
- ABI and flags: `void (Transform *, State *, s32)` under the tail TU's configured `-O2 -mips2 -32 -Wab,-r4300_mul`; target and retained frames are both `0x50`
- callers: one authenticated resident `R_MIPS_26` inbound; no additional resident, local, or cross-overlay caller was found
- oracle: the permitted reference scan found no credible donor; the best masked four-word similarity was 0.031 and unrelated in structure
- baseline: 234/237 words, 162 raw and relocation-masked differences, first `+0x20`, exact `0x50` frame
- retained candidate: 236/237 words, 160 raw and relocation-masked differences, first `+0x20`, exact `0x50` frame; `index = state->selectorA = 3` with a literal second selector store retains the target's narrow-index computation
- relocation proof: target and candidate each carry 13 records; 11 offsets/types align, but all 13 candidate identities remain unresolved in the consolidated TU, so preflight is partial and this is not promotable
- attempts: original and delayed phase carriers plus declaration reordering were byte-flat; removing the wide carrier regressed to frame `0x48` and 166 differences; three selector-chain forms measured 165, 161, and 160 differences; the full chain also regressed to frame `0x58`; a phase register hint and compound pointer advance were byte-flat
- residual: one-word size deficit with mixed early dispatch, transform-copy scheduling, and register allocation; no permutation was run after the ten-source-form cap
- next action: resume only with new evidence for the early phase carrier/return shape, the retained point-base update, or complete consolidated-TU relocation aliases; do not repeat these ten forms
#### B3-o001 (Track B, 2026-09-23): 160 at delta -4 to 30 at delta 0

Cycle 0: `insertion_pairs.py` read three pairs, labelled extra-ISTR and
extra-ILOD; the one that carried the size was a target-only ALU word on the
else-arm z read (line of `obj->z`). The reader's label was a mislabel in the
usual direction: the target keeps the point-element address as a value and
reads z at 4 off it, where the candidate folded `point + 0x14 + 4` into one
displacement. Each measured step, in order:

- rotY stored before rotX/rotZ in both phase-3 arms: 160 to 156 (as1 order of
  the angle load against the two zero stores).
- `point += 0x14` moved above the x read, so x and z both read the advanced
  pointer: 156 to 102, size delta 0. Five other spellings (element pointer
  computed in one expression, as a separate `f32 *`, an inert region, index
  multiply) all stayed folded at delta -4.
- `frame_census.py`: the target's `sp3C` home is 0x3C, ours 0x4C. Declared
  locals all take homes here (L99 corrected); four declared before `sp3C`
  lands it: 102 to 100. Unreferenced pads instead grow the frame to 0x58/0x60.
- `ext_o0_5a914`'s fourth parameter as `f32` (overlay 86 already declares it
  so) passing `0.0f`: the target's `addiu a3,zero,0` form, 100 to 98.
  s16/u8/u16/s8/no-prototype were all 100.
- the fade/alpha update as two statements (`value = x; value -= rate * 4;`)
  puts the load in the value's own register: 98 to 95.
- L160: the narrow index written as the assignment expression itself,
  `(state->selectorA = 3) << 4`, with no `u8 index` carrier: 95 to 30. The
  carrier was taking t0 as a coloured web and rotating the whole temp ring.
  (`index` stays declared unreferenced; it is one of the four homes above.)

What is left (30), priced with the instrumented `uopt` at proc 8, identity
gate passed: the phase/phaseValue webs take v1/v0 where the target has v0/v1
(phaseValue save 3.5 over phase 2.0), and spawned/point likewise. Forcing
`w3=c1,w9=c2,w40=c1,w110=c2` (web numbers from this source's own records)
scores 11. What the forces do not reach: the phase==0 test is `beqz` with the
copy in its delay slot where the target has `beqzl` and the copy in the ==1
test's slot, and two commutative operand orders (`3 == phaseValue` compare,
spawned + offset add) that come out reversed.

Also measured flat or worse on this shape: phase/phaseValue types over
u8/s32/s16/u16 squared with the copy early or late (s32/s32 is 26 but deletes
the target's copy, so it is not the target's shape); `3 == phaseValue`; a
switch over all six phases (jump table, delta -56) or over 2..5 (beq chain,
delta +8).

Next hypothesis: a source form in which the u8-to-s32 copy is in the ==1
test's block (uopt currently places it in the load's block, so as1 puts it in
the ==0 slot) and phase outranks phaseValue; that one shape probably carries
the delay slot and the colour order together.

#### 2026-10-01, lane a-o001: ROM-exact closure, 16 to 0

The inherited shape was the blocker, not the allocator. Three edits, each
measured on the whole TU with the configured flags:

- The phase carrier pair and the volatile re-read are gone. One `u8 phase`
  local is loaded once, tested as `!phase`, then compared against 1 to 5. The
  boolean test reads the variable itself. The comparisons read its promoted
  value, which uopt keeps as a separate expression web and first materialises
  in the block of the first comparison. That reproduces the target's copy in
  that test's delay slot, the branch-likely on the zero test, and the
  constant-first operand order of the compare against the register-held 3.
  Measured: 16 to 12. The `== 0` spelling instead materialises the promoted
  value in the load's block (30), and an `s32` second carrier gives the copy
  but the variable-first compare order (14).
- The route slot is the address of an array element indexed by the selector
  assignment expression, with x and z read as members of a 16-byte element
  type whose array starts at 0x14. The hand-written byte arithmetic with a
  separate add of 0x14 carried a pointer web of total save 5 that out-ranked
  the node pointer (4.5) for the first colour and reversed the address add's
  operands. Forcing those two colours on the old shape priced it at 1; the
  array form scores 0 with no force. The same array form with a `u8` index
  carrier folds the z displacement (delta -4), so the index must stay the
  assignment expression (the L160 finding above still holds).
- `phaseValue` and `index` stay declared and unreferenced. With `value` and
  `phase` they are the four homes above `sp3C`; dropping either moves the
  frame or that home.

Promotion: all seven call sites and the mode global are SYMBOL records, valued
by `gmake overlay-syms` through the existing placeholder names. `gmake verify`
printed the expected SHA1, `check-overlay-syms` and `promotion-proof` passed
(237 words, 13 of 13 relocations). Siblings in the TU re-scored unchanged.

<!-- plateau-handoff:overlay1TransitionState:end -->
