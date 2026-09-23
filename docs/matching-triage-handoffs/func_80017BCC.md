<!-- plateau-handoff:func_80017BCC:start -->
### `func_80017BCC` plateau handoff

- source: `src/main/shadows.c`
- score: 221/314 words
- frame: 0x108
- relocations: 44
- first mismatch: +0x58
- summary: Delta 0 (was +8) via in-place coordinate deltas and per-arm cosine copy; left: 0.0f/var_f0 f20 sharing, sine spill placement

Summary before this remeasure: Frame exact at 0x108; re-measured under the corrected R4300 multiply scheduler, which adds the three FP hazard nops the target carries.
<!-- plateau-handoff:func_80017BCC:end -->
