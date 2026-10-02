<!-- plateau-handoff:func_overlay_055_F000031C_18A1E34:start -->
### `func_overlay_055_F000031C_18A1E34` plateau handoff

- source: `src/overlays/o055/func_overlay_055_F000031C_18A1E34.c`
- score: 0/581 words, promoted
- frame: 0xE0
- relocations: 102
- first mismatch: none
- summary: Matched. Clock address taken after the digit row, item read once at the icon join, the 53 arm's coordinates masked, arm stores and packet words one line each.

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
### Rewritten as overlay 53's sibling, 2026-10-02 (lane/l-o053)

375 masked at delta +4 became 57 at delta 0. Overlay 53's
func_overlay_053_F0000240_189DBE8 matched the same day by being written
as overlay52TailB is written, and this function is its four-player
sibling, so it was rewritten the same way rather than edited: callees and
resident data from this overlay's relocation records with
overlay52TailB's prototypes (u8 colour arguments to func_8002F618 and
func_800016EC and func_800005CC), `player`/`i` loops, `return` (not
`continue`) on a null racer, levelGetLevel called before the
D_800C947C test, the per-player arrays read at each use, the icon's
f32 arguments (0.0f, 0.0f, 0.66f, 0.66f; 0.66f is the float at the end
of this overlay's data), the row pointer `digits` kept because the
shipped code reloads each dividend after the row stores. Priced:

- that rewrite: 375 to 286 at +4.
- the counter increment truncated, `(u8)(counter + 1)` or `& 0xFF`
  (checklist 16, one ring draw): 286 to 275.
- the icon's resource read before the display-list command, and
  func_8002FB34's last parameter u8 (the 1 is then not the s4 constant):
  t0 leaves the ring as the shipped code's item web, 278 to 215 and then,
  with the counter re-swept on the new ring, 60. The `(u8)` truncation
  is the one that stays.
- one unused local declared under gameState (the cells at 0x6C/0x68
  land). The +4 went on the way: `*(s32 *)&digitX` in the digit loop's
  else arm first fixed it (283 at delta 0), and on the final shape plain
  `digitX` in every arm is at delta 0 and best (57 against 78).

Measured flat on this shape (each a product): the lap compare's operand
order, the five x/y store orders around the item test, resource before
or after the zero fields, w0/w1 order, the literal spellings of the f32
arguments, the placement of the `digits` assignment (five positions), the
row expressions as 1-D or 2-D arrays, alpha and item as pointer locals.

Open, 57 words, aligned 31 naming, 4 immediate, 17 structural:

- the clock row's uopt spill cell is 0x5C where the shipped one is 0x60
  (the shipped cells are 0x6C, 0x68, 0x60, 0x54; ours 0x6C, 0x68, 0x5C,
  0x54), so one temporary is created on the other side of the clock row.
  Two words.
- the item pointer: the shipped code reloads it from 0x54 in the block
  after frontGetScreenMode (the bne delay slot), so its coloured piece
  starts after the call and spans the screen-mode diamond; ours reloads it
  at the use, and as1 fills that delay slot with D_800D31C8's high half
  instead. Everything after +0x72C (the item test, the command stores,
  the transition tail) is that one-word shift and its ring phase.
### Matched, 2026-10-02 (lane/m-o055)

57 masked at delta 0 became 0, promoted. Every edit was measured as a cell of
a `tools/shape_product.py` product; numbers are masked words, all at delta 0.

- The clock row's spill cell (0x5C against 0x60): `clock =
  gOverlay55ClockDigits[player]` as a local assigned right after `digits`, in
  place of one unused pad (any declared local takes a frame cell here, so each
  new local replaces a pad). 57 to 55 alone. Assigned before `digits`: 67;
  just before the clock call: 57.
- The item: `item = gOverlay55Items[player]` read once at the icon's join
  (before the x/y stores) and used for both the 53 test and the resource
  index, in place of a second pad. 57 to 45 alone, 43 with the clock local.
  The reload of the spilled item pointer then lands in the screen-mode call's
  delay slot as shipped. Read right after frontGetScreenMode (before the
  branch): 87 at best; through an `&gOverlay55Items[player]` pointer local
  assigned after the call: 64 at best.
- Join stores x before y, and unk08 before unk04: 43 to 39.
- The 53 arm's two coordinates masked, `(iconX - 7) & 0xFFFF` (one ring draw
  each that the store deletes, checklist 16): 39 to 6. Casts `(s16)` instead:
  39/40; `transform.x -= 7`: 40.
- The arm's two stores (x first) on one source line, and the prim-colour
  packet's w0/w1 on one line (L59, as a macro expansion would place them):
  6 to 0. Each alone: 2. `gDPSetPrimColor` itself adds a block-local frame
  cell (13 at best), so the written-out pair stays.

Promotion: mk/overlays.mk renames the resident and overlay 1/56 names to the
`_o055Reloc` surface (overlay56SplitTime was UNRESOLVED otherwise, and the
bare D_800 names would have become global value lines) and externalizes the
0.66f one-constant `.rodata` pool by digest (without it the pool linked into
the overlay and shifted the ROM).
<!-- plateau-handoff:func_overlay_055_F000031C_18A1E34:end -->
