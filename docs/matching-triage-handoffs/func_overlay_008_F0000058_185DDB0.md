<!-- plateau-handoff:func_overlay_008_F0000058_185DDB0:start -->
### `func_overlay_008_F0000058_185DDB0` plateau handoff

- source: `src/overlays/o008/overlay_008.c`
- score: 0/527 words, promoted
- frame: 0xA0
- relocations: 86
- first mismatch: none
- summary: Matched. Rolled selector loop under the default unroller, literal pool, while (index--) walk, no carriers, switch dispatch, float mode-setter argument, shipped frame order.

#### 2026-10-02, lane n-o008: 438 (delta -64) to 0, promoted

Each step measured with `tools/shape_product.py` on the whole TU.

- **The TU's `-Wo,-loopunroll,0` was the -64.** The shipped body reads the
  four selector bytes as a fully unrolled `for (index = 0; index < 4;
  index++)` loop (each byte read twice, constant offsets); the flag had been
  added for the +0x34A0 angle loop and was byte-inert on every function of
  the TU (whole-object compare with and without it). Removed from
  `mk/overlays.mk`. 438/-64 to 448/+16 with the loop; the hand-unrolled
  copies were inherited.
- **D_B0..D_C8 are this function's literal pool** (-31.99, 31.99, 0.98, 0.7,
  -0.1, -0.01, 0.01), retail 0xB0..0xC8: 285/+4.
- **Surface walk** `index = count; while (index--)`: 271/+4.
- **Mode dispatch** is a `switch` with cases 0 and 1 calling the update and
  a default that only converts; the if/else spelled the arms in the other
  order: 174/0.
- **The present flag is read from its global in the test**, not through a
  `present` carrier (one ring draw; the whole residual was one temp-ring
  phase): 28.
- **Frame**: locals declared state, floorHeight, value, one unused word,
  vector, angles, surface, count, index, present, surfaces, update, query: 7.
- **Mode setter** (`o8P0058SpawnReloc`, resident 0x5A914, the same callee as
  o8P34A0SetModeReloc) takes a float last argument; vector stored [2], [1],
  [0]; `position50 == bounce54`; the bounce velocity is re-read from its field
  rather than through `value`: 0.

Promotion: `.rodata` pool externalised by digest with a new rebind spec
(`config/normalizations/overlay8P0058.rebind.spec`, anchor 0xB0); the three
existing pool anchors move down by the new pool's 0x1C bytes (0x17C, 0x198,
0x258).
<!-- plateau-handoff:func_overlay_008_F0000058_185DDB0:end -->
