<!-- plateau-handoff:overlay98CollectUniqueY:start -->
### `overlay98CollectUniqueY` plateau handoff

- source: `src/overlays/o098/overlay98CollectUniqueY.c`
- score: 0/81 words, promoted
- frame: 0x10
- relocations: 8
- first mismatch: none
- summary: Matched. Index the unique list and bound the scan by the count global; name the point-reference index, vertex base and vertex pointer; declare no cursor, end pointer or count copy.

#### 2026-10-01, lane a-ovl: ROM-exact closure

The inherited candidate declared what the object code shows: a walking
cursor, an end pointer, a count copy, a next count and a narrow destination
index. The earlier indexed attempts each kept one of those carriers or
changed the bound, so the cell that matches was never measured: an indexed
scan bounded by the count global itself, with the vertex lookup held in three
named locals.

Measured with the configured flags, direct compile scored with
`tools/score_symbol.py --object`:

- inherited shape: 32 of 81, delta 0
- indexed scan bounded by the global, vertex lookup as one expression: 81,
  16 bytes short and frameless; the scan itself is already the target's, with
  its own address loads for the cursor and the end, but the hoisted addresses
  sit in caller-saved registers
- the same with the point-reference index and the vertex base named: 78,
  4 bytes long
- the same with a named vertex pointer as well: 0 of 81, with the two
  operands of the vertex index in either order and with the append written as
  a post-increment subscript or as a store then an increment
- vertex pointer formed by assignment then addition: 18, register only
- the named-pointer forms with a declared next count: 8 bytes long
- indexed scan bounded by a local copy of the count: over 100 bytes long
  (unrolled)

The three named locals are three more webs in the caller-saved bank, which is
what moves the count address, the list address and the stride constant into
saved registers and gives the function its frame. The recorded declined
forces were facts about the walking-pointer shape.

Proof: overlay 98 text +0x0, 324 executable bytes / 81 words, frame 0x10,
eight of eight relocation identities. The TU is now fully C.

Commands: `gmake overlay-atlas-write`, `tools/refresh_atlas_digest.py`,
`gmake extract`, `gmake overlay-syms`, `gmake verify`,
`gmake check-overlay-syms`, and
`gmake promotion-proof SYMBOL=overlay98CollectUniqueY`.

Identity-gated proc-0 is p2, 18 decisions. Stock and instrumented `.text` are byte-identical. Web 112 is a type-1 address constant coloured s0; its cost table is callee-save only, so `p2:w112=c4` is declined with forced=-2. Web 58 (t3) likewise has no v1 offer. `p2:w58=c11` is accepted and still 32.

The target object (Mickey asm authority) hoists the unique-count address, the constant 10, and the unique-Y base in s0/s1/s2, then rematerializes unique-Y twice more inside the flags path for a forward unique walk versus uniqueEnd. The store uses the hoisted base. That is why a declared unique cursor and uniqueEnd stay load-bearing for the 81-word forward shape.

Lever 50 is a false residual only when raw exceeds masked. Here raw equals masked at 32.
<!-- plateau-handoff:overlay98CollectUniqueY:end -->
