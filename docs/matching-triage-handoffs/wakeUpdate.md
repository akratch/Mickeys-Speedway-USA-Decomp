<!-- plateau-handoff:wakeUpdate:start -->
### `wakeUpdate` plateau handoff

- source: `src/main/fx.c`
- score: 257 differing words
- frame: 0x90
- relocations: 2
- first mismatch: +0x34
- summary: Size delta 0, frame 0x90; residual is p1 colour order (wake/secondaryVertices, index/stripIndex swapped); four colour forces price it at 207.

Summary before this remeasure: JFG efd5abb remains assembly-only; zero source attempts. Need new counter lifetime and trig-call schedule evidence.

Track B lane B3-fx (2026-09-23), about twenty-five measured cycles. Start
367 masked, size delta -4, frame 0x98 against 0x90; insertion reader: five
pairs, aligned residual 310. The retained body was rewritten against the
target; each step with its measured effect:

- Two semantic errors in the old body, read off the target's stores: the
  value stored at vertices-record +0xE is a separate counter (polyCount,
  +2 inside the stripIndex branch, reset with outputCount), not the sample's
  shifted height; and stripIndex advances by 2 per sample (polygon[1] is
  stripIndex, [2] and [0x11] are stripIndex + 1) with polygon[0x12] also
  written in the branch. The shifted height is a plain temporary.
- index as `s32` (the target never masks it), `u16` reads of the linked
  counts, value4 stored without a cast. First version 385 at -12.
- First scan as `count = value3B; while (count--)` with a `start` value for
  the entry and post-scan reads: the target's copy-and-test shape and its
  PRE'd reload. 385 -> 369.
- `index * 5` through a temporary, then `* 4`; sine and cosine as a call
  followed by `*=`. Declarations merged to sixteen slots so the frame is
  0x90 with outputOffset on the target's +0x80 home. 369 -> 335.
- Sample stores in the order angle, +4, arg2, arg1, arg3: 335 -> 329 (best
  of all 120 orders).
- The +0x20 buffer read written as a subscript while +0x18 and +0x28 stay
  pointer arithmetic: the target computes wake + state * 4 twice, and a
  different spelling is what keeps uopt from sharing it. Delta -8 -> 0,
  329 -> 257. Spelling it `<< 2` or `* 4U` measures the same; moving the
  constant after the index does not, and writing all three as subscripts
  shares all three again (329, -8).

Residual at delta 0: byte-exact 146, register naming 232, immediate 1,
really different 23 (four one-sided words each way, all scheduling). The
naming is the p1 colour order, read from the globalcolor records (proc 13):
wake (save 21.86, nocs 14) outranks secondaryVertices (20.17) and takes s3
where the target has s4; the index web (15.38) outranks stripIndex (10.09)
and takes s5 where the target has s6; and polygonOffset (5.17) takes s7,
which the target gives to polyCount, spilling polygonOffset to +0x6C around
the calls instead of our +0x70. Forcing w162=c17 and w2=c18 gives 222;
adding w51=c19 and w12=c20 gives 207, still delta 0. Splitting the
polygonOffset web instead of colouring it moved the size to +8 (296), so
the target's polygonOffset is a caller-saved web that spills, not a split.

Measured flat: a separate symbol for the first scan's cursor (value, mark
or outputOffset: 266, 259, 308), all three buffer reads as subscripts.

Decision variable: the save ratio of the wake parameter against
secondaryVertices, and of the two index webs against stripIndex. The next
lever is whatever lowers wake's totalsave by about two per reference block
(fewer wake reads inside the sample loop) without moving the code.


Reopening audit (2026-09-08), evidence D: PROVENANCE inspection of Jet Force
Gemini public decomp `src/fx.c` and `src/fx.h` at
`efd5abb1c79636e297b831f7c2d5bf47eac39c0c` found no new target C body.
No donor source, names or values were adopted. Mickey remains authoritative.

Configured full-TU measurement: candidate 1584 bytes / 396 words,
target 1592 bytes / 398 words; size delta -8 bytes.
Raw and relocation-masked differences are 334 and 334, respectively,
first mismatch +0x0; candidate/target frames are
0x98/0x90. Candidate/target static relocation counts are
2/2; 1 tuples agree in function-relative offset, type and symbol.
These are fallback-object comparisons, not linked-C promotion evidence.

Workbench comparison: `structure-mismatch`. Diagnosis: `mixed(constant:3, structural:165, register:200)`;
playbook `constant-audit`, lever `stack-home`. The named guides were read.
This heuristic routing supplies no new donor evidence to reopen exhausted forms.
Next concrete lever requires new wakeUpdate donor C exposing counter lifetime and the first trigonometric call schedule,
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

<!-- plateau-handoff:wakeUpdate:end -->
