<!-- plateau-handoff:overlay100DrawMotion:start -->
### `overlay100DrawMotion` plateau handoff

- source: `src/overlays/o100/overlay100DrawMotion.c`
- score: 155 differing words
- frame: 0xC0
- relocations: 7
- first mismatch: +0x0
- summary: L99 unused pointers declared first close frame 0xC0 with exact homes. Packed RGB hoist is plus two words. Command/color lifetime remains.
- assignment base: `b05cf692e3fc02d376d334564fed1d6c1e0a8953`
- owned range: overlay 100 `+0x580..+0x94C`, 972 bytes / 243 words; the following four-byte padding is separately owned
- identity gate: instrumented IDO `.text` byte-identical to stock; `CDX_PROC=0` (47 p1 decisions)
- live: 972 bytes, size delta 0, masked 155, frame `0xC0` matching the target, 15 of 15 stack slots identical including traffic at `+0xC0` / `+0x9C` / `+0x98` / `+0x58`
- aligned: 146 byte-exact, 48 naming, 0 immediate, 69 really different; displacement tax 38
- first mismatch: positional `+0x0`; first naming `+0x58`; first structural `+0x34`
- relocations: seven records (five calls and one HI16/LO16 pair)
- frame close: four unused pointers declared first (L99) grow `0xB0` to `0xC0` at unchanged instruction count. Unused f32s and unused pointers declared last are eliminated. Four projection scalars then the two volatile color homes then `commands` last place every home on the target ladder (161 to 157 to 155; immediate 8 to 0)
- packed RGB hoist: a pre-loop packed color (new local, `progress` reuse, block-scope `register s32`, or in-place `red`) removes the 11-word target-only block at `+0x194` but emits two extra words (980 bytes) and was not adopted
- other lifetime probes: remaining saved in `x` is 968 bytes; remaining plus a `start` cursor is size 0 at 187 masked; empty `if (row) {}` is plus two words. None beat 155
- blocker: command and color lifetime still rebuilds RGB inside the loop and reloads `colorA0` / `colorA1` / remaining / `commands-1` instead of keeping the target's live `a0`/`a1`/`a2`. Next lever is a size-0 spelling of that hoist, not another frame or FP term-order edit
<!-- plateau-handoff:overlay100DrawMotion:end -->
