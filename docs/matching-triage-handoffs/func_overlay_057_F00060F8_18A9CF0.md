<!-- plateau-handoff:func_overlay_057_F00060F8_18A9CF0:start -->
### `func_overlay_057_F00060F8_18A9CF0` plateau handoff

- source: `src/overlays/o057/func_overlay_057_F00060F8_18A9CF0.c`
- score: 0/441 words, promoted
- frame: 0x60
- relocations: 171
- first mismatch: none
- summary: Matched. Rewritten on the F0004E18 template: id walks read the link at each use, countdown fill, direct source and map reads, one player-count word, unrolled reset loop.

## 2026-10-01 (lane `d-o057`): ROM-exact closure, 252 -> 0

The previous closure (two loop-invariant webs, the s16 table base and the
stride 40, must be live across joyCreateMap/mainSetMode/mainChangeCameras)
was true of the inherited shape and dissolved once the shape was rewritten.
Measured with `tools/fast_score.py`, all at the configured flags:

  - The player-setup tail rewritten on overlay 57's matched middle-panel
    function: `count = 10; while (count--)` fill, the choice walk bounded by
    `&sources[index] < &sources[4]` reading `.active` once and the controller
    map twice, the second loop `for (count = playerCount; count < 6; ...)`.
    The relocation records show one resident word (0x3440) read by the
    second loop, mainChangeCameras and the mirror logic, which the candidate
    had split across three names (count, camera, selection); the entrance
    argument is the controller map indexed by the first source, and the
    value08 reset is a `for` over six entries that IDO peels and unrolls into
    the shipped `2 * 40` form. With the inherited link walks this was 279 at
    delta -4.
  - The start and stop walks as `while (link->index != -1)` reading the link
    at each use, with the f32 prototype and `0.007f`: 52 masked at delta 0.
  - Declaration order (240 orders measured): `count, i, link` above the two
    arrays puts them at the target's 0x50/0x44 and the frame at 0x60: 5.
  - `i = 0` folded into the choice loop's for-init (as1 line tie-break):
    0 masked.

Promotion: the overlay 84 and overlay 45 callees are `*Reloc` placeholders;
the seven resident callees are renamed in the object's POSTPROCESS rule.
`gmake verify` printed 507341c0a40ca3e9a7cee969b396ee53facfb548;
`promotion-proof` PASS (441 words, 171/171 relocations).
<!-- plateau-handoff:func_overlay_057_F00060F8_18A9CF0:end -->
