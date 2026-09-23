<!-- plateau-handoff:wakeUpdate:start -->
### `wakeUpdate` plateau handoff

- source: `src/main/fx.c`
- score: 367 differing words
- frame: 0x98
- relocations: 2
- first mismatch: 0x0
- summary: JFG efd5abb remains assembly-only; zero source attempts. Need new counter lifetime and trig-call schedule evidence.


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

Header regenerated from the ranking on 2026-09-23 (check_shard_metrics --write); it read score 334 differing words.
<!-- plateau-handoff:wakeUpdate:end -->
