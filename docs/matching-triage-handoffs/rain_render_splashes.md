<!-- plateau-handoff:rain_render_splashes:start -->
### `rain_render_splashes` plateau handoff

- source: `src/main/weather.c`
- score: 113 differing words
- frame: 0xB8
- relocations: 53
- first mismatch: +0x94
- summary: Size and frame match after JFG while-loop. Remaining 113 is s1-s2-s3 colour (forced floor 44) plus height-result home 0x9C vs 0x84.
- ownership: resident text 0x8003BBF8 through 0x8003C248, 0x650 bytes; the next symbol is `rain_lightning`, so there is no owned tail padding.
- ABI and flags: one `s32` update-rate argument, configured main-TU `-O2 -mips2 -32`; weather POSTPROCESS only renames trap aliases.
- best measured shape: both sides 404 words and frame 0xB8. Masked 113, first mismatch +0x94. Aligner: 291 byte-exact, 93 naming, 3 immediate, 18 structural. One candidate-only word at +0x240 and one target-only word at +0x224.
- height-result home: target 0x84, candidate 0x9C. Four unused function-scope pointers grow the frame 0xA8 to 0xB8 (L99) but do not move that home; extra unused pointers only grow the frame further. Block-scope Gfx command pointers grew the frame to 0xD0 and raised the home.
- colour decision: identity-gated instrumented IDO (proc 14, 41 p1 decisions). Unforced splash web 36 takes s1 (save 76.8). Accepted `CDX_FORCE=p1:w36=c16` (splash to s2) scores 78. Pair `p1:w36=c16,p1:w30=c17` scores 44 at delta 0, putting splash in s2, web 30 in s3, and the 0xFF / D_8007C710 address webs in s1. An L109 or-with-zero probe on D_8007C710 in the render loop and a split spawn/draw splash pointer both regressed.
- do not retry: declaration reordering on the old 340-word shape, countdown alias (407), 119-flag lattice, Gfx block-scope macros, comma-expression vertex writes, cached-age deletion paired with for-loop, unused s32 delay aliases.
- next lever: a source form that makes the splash web take s2 and web 30 take s3 so the 0xFF constant and delay-global address colour s1 without a force (L100 save ranking), then the 0x18 height-result home gap.
<!-- plateau-handoff:rain_render_splashes:end -->
