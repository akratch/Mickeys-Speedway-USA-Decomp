<!-- plateau-handoff:overlay100DrawMotion:start -->
### `overlay100DrawMotion` plateau handoff

- source: `src/overlays/o100/overlay100DrawMotion.c`
- score: 155 differing words
- frame: 0xC0
- relocations: 7
- first mismatch: +0x0
- summary: L99 unused pointers declared first close frame 0xC0 with exact homes. Packed RGB hoist is plus two words. Command/color lifetime remains.
- assignment base: `d8b737b08b810602e750e195d357f542c79f407c`
- owned range: overlay 100 `+0x580..+0x94C`, 972 bytes / 243 words; the following four-byte padding is separately owned
- fresh baseline: 82 of 243 words exact, 161 aligned and raw differences, first mismatch `+0x0`; normalized workbench distance 113
- frame proof: candidate `0xB0` versus target `0xC0`, with exact instruction geometry
- relocation proof: candidate and target each emit seven records, but only 4/7 offsets and types align and no candidate identity resolves
- new mechanism evidence: exact adjacent Overlay 22 commit `73f5946160b3e406ca00449f410a8d74006c9958` proves independent product rotation, explicit negation, and one `(*pointer).field` spelling as IDO allocation levers
- bounded probes: `-(x*cos)+(z*sin)` depth rotation, explicit dereference on the depth x operand, reversed x-projection sum order, and the coherent combined chronology each compile byte-identically to baseline: 82 of 243 exact, 161 differences, `0xB0` frame, distance 113, unchanged relocation surface
- exhausted work: no full flags, prior ten allocation candidates, command/color lifetime forms, or permutation were repeated because none of the four new forms produced a strict gain
- retained source: the prior projection scalar and color-home declaration order remains best unchanged
- blocker: workbench still reports broad structure/allocation mismatch; future reopening requires new source-authentic command/color lifetime evidence, not another FP term-order spelling
<!-- plateau-handoff:overlay100DrawMotion:end -->
