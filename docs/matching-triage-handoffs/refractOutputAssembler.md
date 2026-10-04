<!-- plateau-handoff:refractOutputAssembler:start -->
### `refractOutputAssembler` plateau handoff

- source: `src/main/refractOutputAssembler.c`
- score: 248 differing words
- frame: 0x10
- relocations: 12
- first mismatch: +0x0
- summary: hypothesis=16-word gap is scan/decode/copy scheduling; spellings=loops -260, unrolled scan -140, copy unroll -140; stall=size stays -140, frame 0x10 vs 0x48

#### 2026-10-04: per-slot end checks grow the body and miss the frame

Configured full-TU baseline: 992 target bytes, size delta -140, 213 candidate words, 248 raw and masked words, first mismatch +0x0, frame 0x10 against 0x48. Aligned rows are 1 exact, 79 register-naming, 6 immediate, 175 really different.

Giving each advanced zero-scan slot its own end check before one decode join measures size delta -52, 235 candidate words, and the same 248 raw and masked words, first mismatch still +0x0. Aligned rows are 1 exact, 91 register-naming, 8 immediate, 168 really different. Really-different falls by 7 and the total aligned residual rises from 260 to 267. The frame stays 0x10. The target still touches only the slot at +0x14. The body is not kept. Do not repeat this control shape.

<!-- plateau-handoff:refractOutputAssembler:end -->
