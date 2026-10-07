<!-- plateau-handoff:overlay36UpdateInteractiveEntity:start -->
### `overlay36UpdateInteractiveEntity` plateau handoff

- source: `src/overlays/o036/overlay36UpdateInteractiveEntity.c`
- score: 0/305 words, promoted
- frame: 0x80
- relocations: 24
- first mismatch: none
- summary: Matched. Countdown copy in its own local reused by the marker test (save over the load's), funded by found[9]; promoted, gmake verify passed.

Summary before this remeasure: Record base at entry, timer copy after the test, hit local plus a separately named found[0] reload; left: countdown load v1 and copy v0 swapped.

Summary before this remeasure: Natural rewrite, frame exact, aligned 252/305 exact (was 187); left: countdown/timer split copies, remap hoist, found reload.

Summary before this remeasure: Exact frame from split animation local; countdown CFG carrier remains plus one-instruction size tradeoff.
#### 2026-10-07: natural rewrite, frame exact, 252 of 305 aligned exact

Lane a-ovl3. Baseline 235 masked at size delta 0, aligned exact 187. Rewritten as it reads: the alpha ramp updates `state->alphaA` in place and the scale reads the field (the target's a0 copy and andi reloads come from uopt forwarding the stores, and the whole alpha region then matches), the query fills `Overlay36Found *found[10]` and the animation mode is a scalar `s32` (homes 0x40 and 0x68 as shipped; the 0x6C..0x77 cells are the three scalar locals count, kind and deltaY, which shows every declared scalar takes a cell here), the countdown is carried in `count` before it becomes the loop counter. Result 229 masked at delta 0, aligned exact 252.

Measured and not kept: seven countdown spellings and five timer spellings (all 230); carriers count or kind for any prefix of the countdown and timer uses, or assigned only in the else paths (flat or worse); a register `hit` local with a scalar address-taken `found` plus nine pad words reaches 266 aligned exact but at size delta -4, because uopt then forwards the found load into hit and the target's reload before the callbacks does not appear.

Left, from the aligned diff: (1) the countdown value is v1 for the tests and a v0 copy in the decrement path, and the timer the reverse (v0 tested, v1 copy subtracted); no source copy reaches either, so they read as allocator splits. (2) The target materialises the whole records base in the entry block and the remap flag's high half in its own block; this body hoists the flag's high half instead. (3) The target loads found[0] once into a0 for the deltaY test and both callbacks and reloads it into t6 for the state pointer. Next: dump the p1 records for the countdown web and read why it is split (decision=split), then the found web.
#### 2026-10-07: lane c-ovla, 229 to 4 masked at delta 0

Three edits, each read off an aligned side-by-side of the a-ovl3 body:

- `record = gOverlay36Records;` moved to the entry block (the target materialises the records base before the countdown, and the remap flag's high half then lands in its own block as shipped): 229 to 220.
- Timer: `count = state->timer4;` after the `!= 0` test, the `elapsed >=` compare reading the field and the subtraction reading `count`. The copy is `count` itself (one allocator live range shared with the loop counter, v1), which is exactly the target's `move v1, v0` in the compare's delay slot. Whole timer region exact.
- Found: a `hit` local loaded from `found[0]` used for deltaY and both callbacks (target a0), and the state pointer read through a separately named load of the slot, `*(s32 *)&found[0]`, which uopt does not CSE with hit's load and so reproduces the target's t6 reload. 768-cell product on hit placement and per-use spelling; the plain `found[0]`, `*found`, a pointer cast and `hit->state64` spellings for the state pointer are byte-identical at 15 (uopt CSEs them all). Dropping `hit` (found[0] at every use) is +8 bytes; a hit assigned after its use scored 9 and is semantically wrong.

Left (4 words, +0x48..+0x6C): the countdown. Target loads the countdown into v1, tests it, and copies it to v0 for the 0xFF test and subtraction. The instrumented records (identity-gated, CDX_PROC=0) give the reason: `count` is ONE live range across the countdown copy, the loop counter and the timer copy (bbs 1,4,5,11,12,14,17,19, save 12) and has v0 forbidden by the loop's post-decrement temp (save 20, v0), so it takes v1 and the countdown load web (bbs 0,1, totalsave 4 over nocs 2) takes v0. The target's colours need the countdown copy to be a variable distinct from the loop counter and the timer copy, coloured before the load (save above 2.0) and not spanning the loop. Measured: a fresh `s32 n` for the copy is frame +8 (0x88) and still loses the ranking (save 1.5, load 2.0); `n` with `n -= elapsed` gets v0 but moves the subtraction off t9 (144); `kind` as the copy takes a0 (spans the loop); u8, u32 and s32 copy types are identical; dropping `deltaY` to pay for `n` is +12 bytes. Copy placements before the compare (6), after the early return (5) and the original carried shape (4) were measured with count and kind carriers.
#### 2026-10-07: lane c-ovla, matched and promoted

The 4-word countdown residual priced first: forcing the separate copy local onto c1 (v0) and the countdown load web onto c2 (v1), both accepted (forced=1, forced=2), scores 0 at delta 0. So the residual was that one ranking. The copy local has three references over two save blocks (1.5) against the load's four (2.0). Two facts closed it from source:

- Frame: a fifth declared scalar costs 8 bytes (0x88) at every declaration position (10 positions measured) and in block scope. Declaring the copy local between `animation` and `found` and shrinking `found` to nine elements keeps every home (found at +0x40, animation at +0x68) and the 0x80 frame: L112, the query's array length is not observable.
- Ranking: the same local reused for the marker test (`temp = (s32)entity->marker50; if (temp != 0) ...`) adds references in blocks where v0 is free, lifting its save above the load's, so it takes v0 and the load v1 as shipped. Reusing it for the timer callback pointer scores 0 too; reusing it for the query result does not move the save (uopt forwards the call result in its own block, L105).

`gmake verify` passed after the full promotion sequence; `gmake check-overlay-syms` up to date; `gmake promotion-proof` PASS (305 words, frame 0x80, relocations 24/24).
<!-- plateau-handoff:overlay36UpdateInteractiveEntity:end -->
