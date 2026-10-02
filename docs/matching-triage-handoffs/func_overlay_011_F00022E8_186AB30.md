<!-- plateau-handoff:func_overlay_011_F00022E8_186AB30:start -->
### `func_overlay_011_F00022E8_186AB30` plateau handoff

- source: `src/overlays/o011/func_overlay_011_F00022E8_186AB30.c`
- score: 0/267 words, promoted
- frame: 0x40
- relocations: 79
- first mismatch: none
- summary: Matched. Written as the copy of the matched sibling func_overlay_011_F0001E4C_186A694 with this overlay's relocation identities; first draft exact.

Summary before this remeasure: Exact-size V0 remains 231 raw/227 masked diffs; cursor reuse fixes all three declared stack homes, while the saved-s0 web remains. Linked trial aligns 51/79 sites before a ROM-size stop.
#### 2026-10-02, lane x-o051: 227 to 0, promoted

- The relocation records name the same objects as the matched sibling
  F0001E4C at the same sites (pad index bss+0x1C4, option index bss+0x1BC,
  handles data+0x1CC, highlight data+0x1B8, action data+0x1C4, done
  data+0x204, the resident stick array and configuration bytes), so the
  inherited candidate's `D_menuBase + 0x1C4` volatile cursor and its
  `D_0` reload aliases were reading one object through several names.
- The sibling's body with this function's case bodies (option 2: fixed
  configuration bytes and an unconditional status clear; options 3 and 4:
  level change before the done flag; option 5: release group 6C and set
  the resident next-mode word) scored 0 masked at delta 0 on the first
  compile. The "saved-s0 web" closure described the inherited shape.
- Promotion mirrors the sibling: resident callees renamed to the
  _o011Reloc surface, and the switch table bound to the retained table at
  rodata +0x68 with the compiler's private copy dropped by digest.
<!-- plateau-handoff:func_overlay_011_F00022E8_186AB30:end -->
