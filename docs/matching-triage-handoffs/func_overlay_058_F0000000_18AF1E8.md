<!-- plateau-handoff:func_overlay_058_F0000000_18AF1E8:start -->
### `func_overlay_058_F0000000_18AF1E8` plateau handoff

- source: `src/overlays/o058/func_overlay_058_F0000000_18AF1E8.c`
- score: 0/368 words, promoted
- frame: 0x60
- relocations: 101
- first mismatch: none
- summary: Matched. Rewritten in the matched whale's shape (indexed tables, one symbol per object, count read at each test), 295 to 77; the gap clamp reusing the swap flag instead of its own local took the last 77.

## 2026-10-02 (lane x-o058): matched, 295 to 0, promoted

The inherited candidate was an m2c shape: hand-walked cursors over alias extern names (D_7C, D_94, D_AC, D_C4 for element one of D_78, D_90, D_A8, D_C0, and a separate end symbol for the player-slot table, which the relocation table shows is the same symbol at addend 0xD0), nineteen declared locals, and `register` everywhere. Rewritten from the listing in the whale's vocabulary -- every table indexed by `i`, `D_8007BEF8_o058Reloc` read at each loop test, the whale's race-entry type so the byte at +0x1C is `counters[class]++`, a four-iteration indexed loop for the player slots -- the first compile measured 77 masked at size delta 0, frame 0x68 against 0x60.

The 77 were one local. A separate `gap` for the rank-gap clamp costs two frame cells and leaves initColourCycle's constant 10 at the call; with the clamp written into `swapped` (dead after the rank sort) the frame is 0x60, uopt hoists the 10 into a1 ahead of the gap loop, and the gap and table base take a2 and v1 as shipped. 0 masked, 19 relocation-only words, delta 0.

So the closure above ("six carriers must merge and the surviving positions are pinned") was a statement about the inherited shape: the natural source declares ten locals, not thirteen, and no carrier merge was needed beyond the one reuse. `-Wo,-loopunroll,0` was not load-bearing (the object is byte-identical with and without it) and is removed. Promotion proof: 368 words, frame 0x60, 101 of 101 relocations, identity static. Resident calls and overlay 56's time splitter are renamed on this object's POSTPROCESS.

Summary before this match: Target block solved at 52 bytes, thirteen declarations; six carriers must merge and the surviving positions are pinned.

## 2026-09-11 the target's block solved: thirteen declarations, positions pinned (lane `lane/o11-frames`)

Adopted: a declaration reorder, byte-inert.  295 masked, 109 aligned byte-exact,
165 naming, 18 immediate-only, 87 really-different, delta 0, before and after.
Four choices for which local carries the second position measured identical.

**The frame identity.**  Both objects put the argument build at 0x00..0x17 and
the `s0`/`ra` saves at 0x18/0x1C, and both leave the same twelve bytes of
temporary above them.  So the 24-byte gap is the declaration block and nothing
else: 76 bytes here, 52 in the target.  That is thirteen four-byte declarations
against this candidate's nineteen -- **exactly six**, which sharpens the earlier
note on this page from "five or six cells".

**Where the surviving thirteen sit.**  Reading the target's own slot traffic
against its frame top:

- 1 at -4, spilled once and reloaded once around the two split calls
- 2 at -8, no home traffic
- 3 through 8 at -12 .. -32, the six address-taken out-parameters
- 9 at -36, the order state: stored once right after the first call and read
  back from its home **eight** times
- 10, 11, 12 at -40, -44, -48, no home traffic
- 13 at -52, spilled once and reloaded once around the same two split calls

The adopted order reproduces positions 1 and 3 through 9 of that ladder; 10
through 13 follow once six declarations are gone.

**Two things this changes about the standing description.**

The candidate keeps `state` in a callee-saved register and touches its home
three times; the target does not give it a register at all and re-reads it
eight times.  That is an allocation difference, not a spelling one, and it is
why `state` is ninth in the target's list and eighth here.

The target spills **two** locals around the split-value calls, at the top and
the bottom of its block; this candidate spills only `i`.  The thirteenth
declaration is therefore a second value live across those calls -- a cursor or
class pointer this candidate keeps in an `s` register.

**Probes measured, not adopted.**  Merging `tagSource` into `entry` (301
masked, 103 exact, 171 naming) and `limit` into `count` (294 masked, 105 exact,
168 naming) each take the frame to 0x70 at delta 0; both together also reach
0x70, since seventeen and eighteen cells round the same.  Only thirteen cells
reach 0x60.  Neither merge is evidently the target's, and picking six wrong ones
costs more than the frame buys -- the ladder above is the constraint to solve
against.
<!-- plateau-handoff:func_overlay_058_F0000000_18AF1E8:end -->
