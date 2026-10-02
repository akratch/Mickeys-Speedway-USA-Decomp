<!-- plateau-handoff:shadowGenerate:start -->
### `shadowGenerate` plateau handoff

- source: `src/main/shadows.c`
- score: 432/510 words
- frame: 0x138
- relocations: 63
- first mismatch: +0x118
- summary: Frame exact at 0x138 (unused f32s dropped, homes reordered): 445 to 432 at +40; an s16 still spills to +0xAA where the target keeps type in fp

Summary before this remeasure: Fresh V0 is 520/510 words with 445 differences; frames 0x150/0x138. Both have 63 relocations; 29 sites and identities align. Prior mechanisms closed.

## 2026-10-02 (lane x-shad): frame closed, 445 to 432

The 0x150 frame was five unused f32 locals plus declaration order. A
27-cell product over how many scalars sit above `first` (3 to 5), between
`selected` and the angle arrays (6 to 8) and between the arrays and
`objects` (1 to 3) puts the floor at four, seven and two: frame 0x138 with
`first` +0x124, `selected` +0x120, the arrays +0xEC and +0xDC and `objects`
+0xD0, as in the target. Measured flat or worse on that layout: the
s16 locals widened to s32 (objectType alone, or type and lowAngle
together, moves the frame back off, 445; type or lowAngle alone 432), reading
`object->0x44` at each use instead of the `objectType` carrier (441 at +52).
Left at the head: the target keeps the object in s7, the surface in s6 and
the type in fp, where this candidate has s8, s7 and an s16 home at +0xAA.
<!-- plateau-handoff:shadowGenerate:end -->
