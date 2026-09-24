<!-- plateau-handoff:fxSPDPRipple:start -->
### `fxSPDPRipple` plateau handoff

- source: `src/main/fx.c`
- score: 169 differing words
- frame: 0xA8
- relocations: 12
- first mismatch: +0x68
- summary: extra-ILOD pair was alphaHigh in s8, not the w1 store. volatile alphas and one rippleEnabled load: size 0, 169 words. Stall: lines 2043-2049 address CSE.

Summary before this remeasure: Delta +8 is alphaHigh/alphaLow winning a 2.75 save tie over the hoisted arg3 command temp; splitting both forces delta 0 at 163.

Summary before this remeasure: JFG efd5abb remains assembly-only; zero source attempts. Need new missing relocations and command-loop topology evidence.

Track B lane B3-fx (2026-09-23), about thirty measured cycles. Start 224
masked, size delta +8, frame equal. The m2c body was replaced by a
reconstruction; each step with its measured effect:

- `func_8002A204` prototyped with an `s16` parameter and the three angles
  kept as `s32`: the target sign-extends at each call and increments the
  angles without truncation. With a strength-reduced `for` loop this read
  225 at +12; with explicit angle variables, 226 at +16.
- Frame: declaration order puts alphaHigh 7th and alphaLow 9th (their spill
  homes, +0x8C and +0x84), and four unreferenced `s32` pads take the frame
  from 0x98 to 0xA8. Frame census then agrees on size and on those homes.
- Loop index: no `next` carrier; `(i + 1)` inline in the first command word
  and `i++` / `while (i != arg4)` at the tail. i then outranks the call
  results (save 10.25 against 10) and takes s1, which puts the two call
  results in s2 and s3 as the target has them. 224 -> 208, delta +12 -> +8.
- The arg3 command term as its own expression rather than a reassigned
  parameter: arg3 is then coloured s3 from entry and arg1 is homed, which is
  the target's prologue exactly. 208 -> 205.

Remaining delta +8, measured with the instrumented globalcolor records on
this source (proc 31). The target spends s8 on the loop-invariant command
term `((arg3 & 0x3FF) << 14) or 0xF6000000` and spills alphaHigh and alphaLow
to their homes. Ours gives s8 to alphaHigh: alphaHigh, alphaLow and the two
hoisted command temps all sit at save 2.75 (totalsave 11 over nocs 4), and
the tie goes to the lowest web number, which is alphaHigh. Forcing
`p1:w69=s,p1:w74=s` (both alpha webs split, forced=-1 on both) gives the
target's shape and size delta 0 at 163 masked. Forcing the three address
constant webs to split as well (the target writes the three globals with
`lui at` rather than a materialised base) moved it to 167, so that is a
separate residual.

Measured flat or worse on the tie: statement order of the twelve pre-loop
statements (250 random valid orders, best 204, all still +8); order inside the
wave-sign arms (3 forms); alpha types u8/u16/s16/u32 and `alpha` as u32;
alphaHigh/alphaLow declared in an inner scope; the command term as a declared
local, assigned in the preheader, at the top of the loop or next to its use,
or in two steps (all byte-identical: uopt propagates it back to the temp); a
second local for the arg1 term. Reading level->rippleEnabled into a local
first reaches delta 0 (u8: +4, s32: 0 at 191) but only because the alpha
values then become per-iteration expressions recomputed in the arms, which
the target does not do. `volatile` alphas reach +4 with the command term in
s8, which is diagnostic only. Declaring the three globals as scalars instead
of `[0]` subscripts was byte-identical.

Decision variable: the p1 ranking between the alpha symbol webs and the
hoisted command temp. The next lever is whatever lowers the alpha webs'
save below 2.75 or numbers the temp ahead of them without changing where
they are computed.


Reopening audit (2026-09-08), evidence D: PROVENANCE inspection of Jet Force
Gemini public decomp `src/fx.c` and `src/fx.h` at
`efd5abb1c79636e297b831f7c2d5bf47eac39c0c` found no new target C body.
No donor source, names or values were adopted. Mickey remains authoritative.

Configured full-TU measurement: candidate 936 bytes / 234 words,
target 928 bytes / 232 words; size delta +8 bytes.
Raw and relocation-masked differences are 224 and 224, respectively,
first mismatch +0x8; candidate/target frames are
0xA8/0xA8. Candidate/target static relocation counts are
12/18; 2 tuples agree in function-relative offset, type and symbol.
These are fallback-object comparisons, not linked-C promotion evidence.

Workbench comparison: `structure-mismatch`. Diagnosis: `mixed(constant:1, structural:240, register:49)`;
playbook `constant-audit`, lever `none-known`. The named guides were read.
This heuristic routing supplies no new donor evidence to reopen exhausted forms.
Next concrete lever requires new fxSPDPRipple donor C explaining the six missing relocations and command-loop schedule,
then a fresh gate and configured full-TU comparison.

Stopping evidence: zero source attempts; the authorized donor mechanism supplies
no new implementation. This is the assignment's early-exhaustion stop, not a
five-attempt stall. The prior TU lattice is not repeated: since its recorded
`func_8004ACC4` audit at `4be95a3d`, this TU differs only in EOF handoff comments.
See [the anchor donor audit](func_80049E4C.md) for the full delta and batch disposition.

Ignored evidence is retained in `build/tu-fx-audit/` and `build/wb/`: source and
full-TU object, gate receipts, summaries, diagnoses and relocation tuples.
Commands: `lane_status.py --symbol`, `wb_compare.sh --summary-json`, workbench
`diagnose` and `guide`, and `finalize_plateau.py`. The unchanged guarded C stays
NON_MATCHING and receives zero new exact bytes.

w4-fx remeasure, stock IDO only. insertion_pairs on the +8 body named pair 6
(extra-ILOD, shadow 15) as a store on the second-packet w1 line and a load of
`*dList` on the next line. The opcode multiset said those were alignment
shadow: the real extras were two `or`s, `move` of alphaHigh into s8 at its
definition and `move` of s8 into alpha in the positive arm, while cmdHi took
the stack slot the target uses for alphaHigh.

Spellings that moved the score:
- volatile alphaHigh and alphaLow together: size +8 to +4, cmdHi in s8, spills
  at 0x8C and 0x84, one extra `lbu` of rippleEnabled because the volatile store
  splits the field load.
- that field loaded once into the existing `next` local, and two unused s32
  pads removed: size 0, frame 0xA8, 169 masked, first mismatch +0x68,
  12 relocations. alphaLow left non-volatile takes s8 and the size returns
  to +4.

Stall: the only opcode-multiset gap is three `addiu` where the target has
three `lui`. Lines 2043-2049 keep each global's address in a register across
the halfword store. Scalar declarations, a volatile cast at the use, and
volatile scalar declarations were byte-identical; the address CSE survived
all three. No further size or masked improvement.

<!-- plateau-handoff:fxSPDPRipple:end -->
