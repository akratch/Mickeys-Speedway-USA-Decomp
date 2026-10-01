<!-- plateau-handoff:rain_render_splashes:start -->
### `rain_render_splashes` plateau handoff

- source: `src/main/weather.c`
- score: 109 differing words
- frame: 0xB8
- relocations: 53
- first mismatch: +0x94
- summary: 109 with the height-result home at the target 0x84 and a for render loop; the s1/s2/s3 ring is one ranking: delay address and 0xFF outrank splash and index.
- ownership: resident text 0x8003BBF8 through 0x8003C248, 0x650 bytes; the next symbol is `rain_lightning`, so there is no owned tail padding.
- ABI and flags: one `s32` update-rate argument, configured main-TU `-O2 -mips2 -32`; weather POSTPROCESS only renames trap aliases.
- best measured shape: both sides 404 words and frame 0xB8. Masked 113, first mismatch +0x94. Aligner: 291 byte-exact, 93 naming, 3 immediate, 18 structural. One candidate-only word at +0x240 and one target-only word at +0x224.
- height-result home: target 0x84, candidate 0x9C. Four unused function-scope pointers grow the frame 0xA8 to 0xB8 (L99) but do not move that home; extra unused pointers only grow the frame further. Block-scope Gfx command pointers grew the frame to 0xD0 and raised the home.
- colour decision: identity-gated instrumented IDO (.text identical to stock; proc 14, 41 p1 decisions; unforced records forced=-2). Unforced splash web 36 takes s1 (save 76.778, nocs 9, totalsave 691). Web 30 takes s2 (save 56.833, nocs 6, totalsave 341). Type-2 0xFF web 233 takes s3 (save 40, nocs 4, totalsave 160). D_8007C710 address web 211 also takes s3 (save 8.857, spawn-only). Accepted `CDX_FORCE=p1:w36=c16` scores 78 at delta 0, but web 30 then takes s1 rather than s3. Pair `p1:w36=c16,p1:w30=c17` both accepted (forced=16 and 17) scores 44 at delta 0, putting splash in s2, web 30 in s3, and the 0xFF / D_8007C710 address webs in s1. The 44-word unforced ranking is therefore 0xFF save greater than splash 76.8 greater than web 30 56.8.
- L160 spawn: the target inner search conditionally advances the cursor (else splash plus-equals 1) with a countdown index. Indexed `splash = &D_8007C3E4[index]` plus `countdown = index++` in a do-while is size delta -4 (403 vs 404), naming 60 vs 93, frame still 0xB8. Adding the else-increment is size delta +4. A while-form of the same walk is +24 to +32. Direct `D_8007C3E4[index]` stores are +44. Strength reduction cannot emit the target's conditional increment, so indexed spawn does not close size.
- L160 render: `splash = &D_8007C3E4[index]` in the always-increment draw loop is size delta -8 and drops a frame slot (17 vs 18).
- L100 / L109: discarded or-with-zero on D_8007C710 in the spawn while and discarded or-with-zero on 0xFF in the render loop are CSE'd; totalsave of webs 211 and 233 unchanged. Or-equals-zero on D_8007C710 as a store is +4. Or-equals-zero on updateRate in the render loop is size 0 and does not move web 30 totalsave (already read in that loop). Carrying 0xFF in already-used temp with or-equals-zero keeps size/frame 0 but scores 209; the 0xFF web becomes type-3 web 54 at save 46.2, still below splash 76.8.
- do not retry: declaration reordering on the old 340-word shape, countdown alias (407), 119-flag lattice, Gfx block-scope macros, comma-expression vertex writes, cached-age deletion paired with for-loop, unused s32 delay aliases, L109 or-with-zero on D_8007C710 in the render loop, split spawn/draw splash pointers, indexed spawn walk (conditional-increment size gap), indexed render walk, discarded D_8007C710 or 0xFF or-zero probes, updateRate or-equals-zero in the render loop, temp-as-0xFF carrier.
- next lever: a zero-size way to raise the type-2 0xFF web above splash's 76.8 save (L100) so splash takes s2 and web 30 takes s3 unforced; lowering splash alone yields the 78-word colouring (web 30 takes s1). Then the 0x18 height-result home gap.

## 2026-10-01 (lane d-res1): 113 to 109, and what the ranking needs

- The height-result home is a declaration slot: it is the 13th declared local
  in the target (home 0x84) and the 7th in the retained candidate (0x9C), so
  moving that declaration to the 13th position (the unused pointer pads stay
  ahead of it) puts the home right and saves three words (112 to 109) with no
  change in size or frame. `heightResult` above that position, and every
  other position, scored 112.
- The render loop as `for (index = 0; index < 0x10; index++, splash++)` is 112
  against 113 for the do-while; the spawn search loop forms (while, for,
  `!found`, swapped arms) and initialiser orders (all six) were flat or worse.
- Flat in a product: a separate render pointer or counter (112, 143), a `white`
  local for the sixteen 0xFF stores (112), the fully indexed render loop (394
  at delta -8, the frame shrinks because the pointer web disappears).
- The p1 ladder (proc 14) orders webs by save per block: 0xFF stores 40, splash
  76.8, index 56.8, delay address 8.9. The target's s1 is shared by the delay
  address and the 0xFF constant, s2 is splash and s3 the index, so the target
  colours both small webs before splash and index. Splash and index would need
  a total below the delay address's 8.9 per block, or the delay address a total
  over 20.6 (density, which takes s4 in the target). Neither was reached from
  source; the forced pair from the earlier lane (44) is the reference.

<!-- plateau-handoff:rain_render_splashes:end -->
