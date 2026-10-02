<!-- plateau-handoff:func_overlay_043_F0000BE4_188ABB4:start -->
### `func_overlay_043_F0000BE4_188ABB4` plateau handoff

- source: `src/overlays/o043/func_overlay_043_F0000BE4_188ABB4.c`
- score: 0/305 words, promoted
- frame: 0x158
- relocations: 29
- first mismatch: none
- summary: Matched. Rewritten from the listing as GBI packet macros on the address-taken cursor; inherited -mips1 override removed; display-list start held in a register local and copied into the cursor; model read before the vertex data; list end stored before the pending flag; one unused local below the matrices.

Summary before this remeasure: 307 differing words at size delta +28, frame 0x158, 29 relocations against the shipped 22, first mismatch +0x4.

#### 2026-10-02, lane w2-capbuf: matched and promoted (307 at +28 to 0)

Rewritten from the listing, not from the inherited m2c shape (which carried a
padded locals struct, a walking byte offset and an `OVERLAY43_NEXT_GFX`
carrier per command). The relocation records name the callees: resident
`func_80044BC8` (diRcpTrace, called with the file name and lines 596 and
675), `rsp_segment`, `rcpInitDpNoSize`, `mtxf_mul` (three calls) and
`mtxf_to_mtx`. The measured steps, masked words at size delta:

- Natural rewrite: one GBI macro per command on `dl++`, a counted
  `for (i = count - 1; i >= 0; i--)` over `entries[i]`, the K0-to-physical
  conversion as libultra's `(char *)x - 0x80000000`: 268 at 0.
- F3D movewords encoded as `gImmp21` (offset in bits 8-15, index in 0-7),
  the alpha local widened to s32 (a u8 local emits `lbu 0x47` where the
  target loads the word and masks), the vertex-data and model reads moved
  above the queue-count test, and the per-file `MIPSISET := -mips1 -32`
  override removed: 226 at 0. The target fills no load-delay slot
  (`lw t8` then `lw 0x34(t8)`), so it is MIPS II, the overlay default.
- Frame: every declared scalar takes a home in declaration order here, and
  the target's ladder (cursor 0x154, vertex data 0x140, state 0x13C,
  matrices 0xFC/0xBC) needs seven scalars above the matrices and two cells
  below them: 220 at 0 with the frame exact. The aligner then read 204
  naming rows as one coherent eight-register ring cycle (L127), one draw
  ahead of the target.
- The draw: `dl = state->displayList; trace(dl, ...)` spends a ring
  register on the load. The target holds the start in a register local
  (`start = state->displayList; ...; dl = start; trace(start, ...)`),
  which globalcolor puts in a0 at no draw: 17 at 0. Four other spellings
  of the same assignment (inside the argument, the trace given the field,
  the copy after the call) measured 218 to 286.
- A 12-cell product over the entry order (model before vertex data) and the
  tail order (`displayListEnd` before `pending`): one exact cell.
- The queue count is its own symbol: written as `.data` base + 0xC8 the
  compiler folds 0xC8 into the `lb`/`sb` displacement, while the target
  materialises the full address and reads at 0.

Promotion: `*Reloc` placeholders for the five resident callees and the
module-relative bases; overlay-syms resolved them with no rename rules.
`gmake verify` OK, `check-overlay-syms` up to date, promotion-proof PASS
(305 words, relocations 29/29).
<!-- plateau-handoff:func_overlay_043_F0000BE4_188ABB4:end -->
