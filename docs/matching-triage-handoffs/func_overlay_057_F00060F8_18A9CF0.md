<!-- plateau-handoff:func_overlay_057_F00060F8_18A9CF0:start -->
### `func_overlay_057_F00060F8_18A9CF0` plateau handoff

- source: `src/overlays/o057/func_overlay_057_F00060F8_18A9CF0.c`
- score: 252/441 words
- frame: 0x60
- relocations: 175
- first mismatch: +0x7C
- summary: The +2 ring offset is two loop-invariant webs, the s16-table base and the constant 40 menu-entry stride, which hold t0 and t1 here and are callee-saved s1 and s2 in the target because there both are still live after joyCreateMap mainSetMode and mainChangeCameras: s1 reaches the mainChangeLevel entrance argument off the same address as the controller map, and s2 is multiplied by 2 in the value08 block; so the variable is cross-call liveness, not indexed versus walked, and every form tried so far is either folded back to constant offsets by uopt or costs size. Adopted here: the third layout branch's last argument is 4 not 0x104, and the second loop is a for over the count which reproduces the target's strength reduction, taking 256 to 252 at delta 0.

Header regenerated from the ranking on 2026-09-23 (check_shard_metrics --write); it read first mismatch +0x14.
<!-- plateau-handoff:func_overlay_057_F00060F8_18A9CF0:end -->
