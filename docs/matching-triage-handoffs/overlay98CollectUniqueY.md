<!-- plateau-handoff:overlay98CollectUniqueY:start -->
### `overlay98CollectUniqueY` plateau handoff

- source: `src/overlays/o098/overlay98CollectUniqueY.c`
- score: 32/81 words
- frame: 0x10
- relocations: 8
- first mismatch: +0x68
- summary: Lever 51 stall. Named uniqueEnd is required for 81 words and three stack slots; unique indexing without it is 78-80, with an extra s32 index +26. Span subscripts are inert; dropping the block pointer grows to 88. Identity-gated proc-0 is p2, 18 decisions; the s0 web cannot take a1. Aligned 52 exact, 19 naming, 7 structural plus 3/3 insertions. Best remains the walking unique scan.
<!-- plateau-handoff:overlay98CollectUniqueY:end -->
