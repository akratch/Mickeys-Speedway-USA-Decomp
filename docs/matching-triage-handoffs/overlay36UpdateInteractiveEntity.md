<!-- plateau-handoff:overlay36UpdateInteractiveEntity:start -->
### `overlay36UpdateInteractiveEntity` plateau handoff

- source: `src/overlays/o036/overlay36UpdateInteractiveEntity.c`
- score: 229 differing words
- frame: 0x80
- relocations: 24
- first mismatch: +0x2C
- summary: Natural rewrite, frame exact, aligned 252/305 exact (was 187); left: countdown/timer split copies, remap hoist, found reload.

Summary before this remeasure: Exact frame from split animation local; countdown CFG carrier remains plus one-instruction size tradeoff.
#### 2026-10-07: natural rewrite, frame exact, 252 of 305 aligned exact

Lane a-ovl3. Baseline 235 masked at size delta 0, aligned exact 187. Rewritten as it reads: the alpha ramp updates `state->alphaA` in place and the scale reads the field (the target's a0 copy and andi reloads come from uopt forwarding the stores, and the whole alpha region then matches), the query fills `Overlay36Found *found[10]` and the animation mode is a scalar `s32` (homes 0x40 and 0x68 as shipped; the 0x6C..0x77 cells are the three scalar locals count, kind and deltaY, which shows every declared scalar takes a cell here), the countdown is carried in `count` before it becomes the loop counter. Result 229 masked at delta 0, aligned exact 252.

Measured and not kept: seven countdown spellings and five timer spellings (all 230); carriers count or kind for any prefix of the countdown and timer uses, or assigned only in the else paths (flat or worse); a register `hit` local with a scalar address-taken `found` plus nine pad words reaches 266 aligned exact but at size delta -4, because uopt then forwards the found load into hit and the target's reload before the callbacks does not appear.

Left, from the aligned diff: (1) the countdown value is v1 for the tests and a v0 copy in the decrement path, and the timer the reverse (v0 tested, v1 copy subtracted); no source copy reaches either, so they read as allocator splits. (2) The target materialises the whole records base in the entry block and the remap flag's high half in its own block; this body hoists the flag's high half instead. (3) The target loads found[0] once into a0 for the deltaY test and both callbacks and reloads it into t6 for the state pointer. Next: dump the p1 records for the countdown web and read why it is split (decision=split), then the found web.
<!-- plateau-handoff:overlay36UpdateInteractiveEntity:end -->
