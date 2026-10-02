<!-- plateau-handoff:func_overlay_073_F0000190_18CAC50:start -->
### `func_overlay_073_F0000190_18CAC50` plateau handoff

- source: `src/overlays/o073/func_overlay_073_F0000190_18CAC50.c`
- score: 755 differing words
- frame: 0x98
- relocations: 48
- first mismatch: +0x10
- summary: Fresh current-base frame-exact 760/766 result; promotion stops at relocation schedule divergence (48/46 sites), so close extent before allocator/permuter work.

Header regenerated from the ranking on 2026-09-23 (check_shard_metrics --write); it read first mismatch +0x0.

#### 2026-10-02, lane x-sib2: first notes after the sibling Draw matched

`func_overlay_073_F0000D70_18CB830` matched as a sibling copy of the overlay
71 renderer (packet macros, `vertices[vertexBank * 6]` over ten-byte
vertices); this updater writes the same vertex banks.

- `D_20` through `D_54` are LOCAL records against the module's rodata (data
  +0xD0); they are float literals (0.004, 0.1, 0.064, 22500.0, 1.2, 1.6).
  Writing them as literals is inert (755 at +24) but is the shape the
  promotion needs.
- Entry: the target re-sign-extends the 16-bit field at state+0x94 before
  the multiply by `updateRate` (a cast the candidate lacks), keeps the
  state pointer in a temporary spilled at +0x58, and in case 0 stores the
  zero timer and passes the zero float argument from two separate
  materialisations (the candidate shares one).  The hits buffer is at
  +0x38 in the target (+0x90 in the candidate), so the local set and order
  differ from the first block on.
<!-- plateau-handoff:func_overlay_073_F0000190_18CAC50:end -->
