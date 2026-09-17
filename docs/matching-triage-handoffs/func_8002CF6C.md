<!-- plateau-handoff:func_8002CF6C:start -->
### `func_8002CF6C` plateau handoff

- source: `src/main/saves.c`
- score: 54/88 words
- frame: 0x48
- relocations: 11
- first mismatch: +0x20
- summary: Stack-homed SavesWipeState matches the 0x48 frame and slot ladder. 86 vs 88 instructions. Two target-only words remain: a DCE'd addiu of the buffer copy by 0x1C0, and a nop in the mainResetPressed jal delay.
<!-- plateau-handoff:func_8002CF6C:end -->
