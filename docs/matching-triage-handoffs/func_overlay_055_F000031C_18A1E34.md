<!-- plateau-handoff:func_overlay_055_F000031C_18A1E34:start -->
### `func_overlay_055_F000031C_18A1E34` plateau handoff

- source: `src/overlays/o055/func_overlay_055_F000031C_18A1E34.c`
- score: 375 differing words
- frame: 0xE0
- relocations: 90
- first mismatch: +0x1D0
- summary: 445 to 375: slot-exact declaration order, three alias locals read as expressions; temp-ring and spill cells open

Summary before this remeasure: V0: 575/581 words, exact 0xE0 frame, 445 raw diffs. Relocs 90 versus 102; 34 sites and 20 identities align. Display/transition lifetime web persists.

Header regenerated from the ranking on 2026-09-23 (check_shard_metrics --write); it read first mismatch +0x2C.
### Slot-exact declaration order, 2026-10-02 (lane/f-mixed)

445 masked at delta -24 became 375 masked at delta +4 (one word over). The
object already carried `-Wab,-r4300_mul`; the loops needed nothing further.

- Declaration order. Declared homes are allocated from the top of the locals
  area downward in declaration order, and the shipped frame leaves 11 unused
  words around its memory-resident locals (two above the digit offsets, four
  above the time counters, five between the height offset and the transform).
  A random search of the register-only declarations over those groups (about
  240 compiles) put every memory slot above 0x78 on the shipped offsets and took
  the size delta from -24 to 0 before the alias edits: 445 to 440.
- `player` (object->state) read as an expression instead of a local: 440 to
  383 measured alone. Reading the icon cell and the placement the same way and
  adding one unused `s32` pad to keep the frame at 0xE0: 383 to 375.
- Measured and worse: quotient/remainder temporaries for the digit pairs (the
  shipped code divides six times, each with its zero check, so they regress to
  452 at delta -152); keeping `player` as a local (429 to 438 over 20 subset and
  order cells).

Open: the spill cells in the lower temp area (the candidate uses 0x58/0x60/0x64
where the shipped code uses 0x50/0x54/0x60/0x68), and the temp-ring phase. The
shipped code holds the player state in a saved register and the icon cell in a
spill slot, which the expression forms above cannot reproduce.
<!-- plateau-handoff:func_overlay_055_F000031C_18A1E34:end -->
