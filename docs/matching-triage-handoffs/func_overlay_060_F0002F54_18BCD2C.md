<!-- plateau-handoff:func_overlay_060_F0002F54_18BCD2C:start -->
### `func_overlay_060_F0002F54_18BCD2C` plateau handoff

- source: `src/overlays/o060/func_overlay_060_F0002F54_18BCD2C.c`
- score: matched (0/222 words, promoted 2026-09-16 by lane s1-c)
- frame: 0xC0
- relocations: 12
- first mismatch: none
- summary: Matched at delta 0 on four source edits and no force: a block-scoped macro for the first append, SDK-style masks on the u8 colour bytes (folded draws), a strength-reduced x - left, and the declared-local set that homes color at 0x9C.
#### 2026-09-13, lane h1: redundant offset occurrence differential

Fresh configured baseline: 222 words, 888 bytes, frame 0xC0, 84 raw/masked
differences, first +0x64. All 24 observed stack slots and traffic agree.
Paired aligned buckets are 138 exact, 75 naming, zero immediate and eight
structural rows, plus one candidate-only and one target-only word. Static
relocations are 12 candidate and six extracted target records, with one
matching tuple; these counts do not resolve overlay runtime identities.
The census records 74 draws and 300 emission records, and full-TU stock/traced
section, symbol and relocation fidelity passes.

The recorded display-address/x exchange and prior order controls were read.
One source probe replaces offset = x - left with zero. At that point x was
just assigned left and neither value changed, so the probe preserves values
and every access/call position. It asks whether the otherwise redundant x
occurrence controls the saved-web ordering or emitted schedule.
All line counts and the draw sequence remain identical. Three object words
change at +0x13C, +0x140 and +0x150, but the raw/masked score, all paired
aligned bucket totals and every per-window count remain unchanged. This is
not byte identity, and the counter-occurrence form supplies no residual gain.

Restore the guarded baseline and stop early under ADR 0018 for this route,
alongside the recorded declaration/order controls. The address/x ordering and
later temporary schedule remain unresolved; no general impossibility is claimed.
No colour sweep was repeated and no bytes are credited. Private evidence is
under build/h1/func_overlay_060_F0002F54_18BCD2C. Commands: configured compile,
draw_census.py --save/--compare, residual_map.py --object/--against, workbench
fidelity, direct owned-range comparison, finalize_plateau.py and tools/gates.sh.

#### 2026-09-16, lane s1-c: matched and promoted (0/222 words, delta 0, frame 0xC0)

Closed from 84 in six measured cycles with no colour forced; `gmake verify`
prints the expected SHA1 from the C with GLOBAL_ASM gone. Each step was
measured alone on a private direct-cc harness that reproduced the score of
record (84, first +0x64) before use, and the adopted body scores 0 through
`tools/score_symbol.py`.

- 84 to 78: the first append spelled through a one-line macro with a
  block-scoped pointer, the loop's three appends left on the declared
  `command`. The target loads three appends into v0 and the loop's last into
  v1; one declared symbol web cannot do that, and the shard's "display-list/x
  web" reading was this in disguise. Full 3^4 lattice over the four sites:
  every form with two or more block-scoped pointers moves color's home
  (L99: every declared local, live or dead, takes a slot in declaration
  order, block-scoped ones last).
- 78 to 19: the freelist trace (DKWB_UGEN_SCHED) read against the target's
  arm-1 registers says the target spends one invisible draw around each
  colour-byte lbu, drawn before the load and freed before the shift. That
  is L149's packInit generator: the SDK's `_SHIFTL` mask on a u8 field,
  which as1 folds into the lbu after ugen has drawn its temp. Lattice of
  32 mask placements: arm 1 needs red, green and blue masked (alpha inert),
  arm 2 needs red only (the mask under `<< 24` of a shifted byte is the one
  uopt does not remove first); one-line and multi-line layouts identical.
- 19 to 7: the residual was one s0/s1 exchange, x against the display-list
  address web, and the records priced it: address web 62/3 = 20.67 against
  x 61/3 = 20.33. Deleting the `offset` local and writing
  `(f32)(x - left)` inside the loop (strength-reduced to the same s2
  induction variable) flips it; `offset = 0`, a guard on x, `x = x + 4`,
  split from `(f32)x` and `offset = offset + 4` are all inert at 19, a
  `for` form is +4, and recomputing `offset = x - left` inside the loop
  grows the frame.
- 7 to 0: the seven immediate rows were color's home at 0xA0 instead of
  0x9C. Read off the -g3 .mdebug table (a 30-line reader, private): every
  declared local takes a 4-byte slot descending from the frame top in
  declaration order, block-scoped pointers after the function-scope ones,
  and the frame is 0x70 + round8(declared + a temp reserve) where the
  reserve is 24 in the declared-offset form and 28 once uopt strength-
  reduces `x - left`. So the reduced form needs six slots above color and
  one fewer below at frame 0xC0. Of 64 cells, three are exact: a declared
  `axis` local (s16 or s32) for the input-axis value the target holds in
  v0 across both tests, or a second display-list pointer for the fill
  rectangle, each with `topFloat` inlined. The `s16 axis` form is adopted.

Promotion: the C called `func_overlay_082_F00004A4_18CF624` by name and
`gmake verify` failed by exactly two bytes at 0x18BCD86 with the function
at 0 masked -- the cross-overlay `jal` linked as a JUMP encoding of the
callee's overlay-82 offset (`0c000129`) where the ROM stores the
`0xF0000000` addend (`0c000000`). The fix is CLAUDE.md's: name the callee
by its `overlay82IsActive_o060Reloc` placeholder, as overlay60Prefix's rule
does. `reloc_surface.py` rewrote the three resident calls and said nothing
about this one. After that, `check-overlay-syms` up to date and
`gmake verify` prints the expected SHA1.

<!-- plateau-handoff:func_overlay_060_F0002F54_18BCD2C:end -->
