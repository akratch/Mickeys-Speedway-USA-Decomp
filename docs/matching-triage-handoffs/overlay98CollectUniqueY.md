<!-- plateau-handoff:overlay98CollectUniqueY:start -->
### `overlay98CollectUniqueY` plateau handoff

- source: `src/overlays/o098/overlay98CollectUniqueY.c`
- score: 32/81 words
- frame: 0x10
- relocations: 8
- first mismatch: +0x68
- summary: Lever 51 stall. Indexed unique scan unrolls +26 or 78-80 without uniqueEnd; != uniqueEnd SRs to walking 32. Countdown 31/0 is backward. p2 w112 cannot take a1.

Identity-gated proc-0 is p2, 18 decisions. Stock and instrumented `.text` are byte-identical. Web 112 is a type-1 address constant coloured s0; its cost table is callee-save only, so `p2:w112=c4` is declined with forced=-2. Web 58 (t3) likewise has no v1 offer. `p2:w58=c11` is accepted and still 32.

The target object (Mickey asm authority) hoists the unique-count address, the constant 10, and the unique-Y base in s0/s1/s2, then rematerializes unique-Y twice more inside the flags path for a forward unique walk versus uniqueEnd. The store uses the hoisted base. That is why a declared unique cursor and uniqueEnd stay load-bearing for the 81-word forward shape.

Lever 50 is a false residual only when raw exceeds masked. Here raw equals masked at 32.
<!-- plateau-handoff:overlay98CollectUniqueY:end -->
