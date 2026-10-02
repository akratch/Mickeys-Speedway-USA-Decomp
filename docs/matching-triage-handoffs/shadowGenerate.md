<!-- plateau-handoff:shadowGenerate:start -->
### `shadowGenerate` plateau handoff

- source: `src/main/shadows.c`
- score: 432/510 words
- frame: 0x138
- relocations: 63
- first mismatch: +0x118
- summary: Frame exact at 0x138 (unused f32s dropped, homes reordered): 445 to 432 at +40; an s16 still spills to +0xAA where the target keeps type in fp

Summary before this remeasure: Fresh V0 is 520/510 words with 445 differences; frames 0x150/0x138. Both have 63 relocations; 29 sites and identities align. Prior mechanisms closed.
<!-- plateau-handoff:shadowGenerate:end -->
