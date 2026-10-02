<!-- plateau-handoff:func_overlay_061_F0000B84_18BFF4C:start -->
### `func_overlay_061_F0000B84_18BFF4C` plateau handoff

- source: `src/overlays/o061/func_overlay_061_F0000B84_18BFF4C.c`
- score: 0/637 words, promoted
- frame: 0x70
- relocations: 264
- first mismatch: none
- summary: Matched. Rewritten from the listing: sound calls the m2c candidate dropped, six separate sprite/animation/time objects, size reusing the loop index, locals declared i, outputs, result, inputs, alphas, type cursor; u8 alpha parameters of func_8002F618 make the shipped re-read of each alpha.

Summary before this remeasure: 476 masked words at size delta -64, frame 0x78 against 0x70, first mismatch +0x0.

#### 2026-10-02, lane w2-ovle: matched and promoted (476 to 0)

The inherited candidate was an m2c shape missing whole statements: the
confirm/cancel sound calls in cases 4, 5 and 6, and the second sprite's frame
update. Written from the listing it measured 133 at -32. The cells after
that, in order (shape_product, masked words at size delta):

- Frame homes: the target's ladder has one otherwise unused word above the
  three pak outputs and one between them and the four input words. Declaring
  i first and result between the outputs and the inputs, with no separate
  size or record local (size reuses i, so the root and the loop counter are
  one s0 web as shipped), lands frame 0x70 and every home: 67 at -32. The
  loop test also became the shipped slti once i was declared there.
- The two sprites, their animation words and their times as six objects
  instead of one 0x28-byte struct array: the time and frame fields are then
  absolute addresses and only the first sprite's base is an s0 web: 38 at -12.
- The last 12 bytes are the re-read of each alpha after its test. A
  volatile declaration reproduces the re-read (12 at 0) but pins the case
  stores; address-taking, an array, cast reads and same-line placement do
  not. func_8002F618's real prototype (src/main/rcpFast3d.c) takes the
  colour and alpha as u8: the narrowing conversion makes the argument a
  different expression from the test, and the cell is exact.

Promotion: the 9-entry switch table is the retained table at rodata +0x188
(data_rodata +0x278), bound by a rebind spec and externalized by digest
(overlay 57's form); the overlay 45 mode setter is the overlay61SetModeReloc
placeholder; seven resident callees go through the _o061Reloc surface.
<!-- plateau-handoff:func_overlay_061_F0000B84_18BFF4C:end -->
