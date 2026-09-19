<!-- plateau-handoff:overlay13DrawActive:start -->
### `overlay13DrawActive` plateau handoff

- source: `src/overlays/o013/overlay13DrawActive.c`
- score: 24 differing words
- frame: 0x168
- relocations: 10
- first mismatch: +0x40
- summary: Size and 0x168 homes exact. Unforced 24 naming. FP half closed by early temporaryDistance def. record still keeps v1 over a0. Force-0 is not a match.

- geometry: Overlay 13 `+0x874..+0xB0C`, 664 bytes / 166 words both sides, size delta 0.
- ABI/flags: Overlay 13 `-O2 -mips2 -32`. Frame `0x168` both. Slot ladder identical, 11 of 11. Save set `ra` and `s0` only.
- aligned residual: 142 byte-exact, 24 register naming, 0 immediate, 0 structural, displacement tax 0. First naming `+0x40`.
- identity gate: instrumented IDO `.text` is byte-identical to stock. `CDX_PROC=0`.
- unforced FP close: `temporaryDistance = 0.0f` at function start numbers that type-3 FP web first (web 0, colour 24). The sort's type-4 FP temp then takes colour 25. That is the old `p1:w76=c25` 14-word force, now ordinary source. Size and frame unchanged.
- remaining integer cycle: walking `record` (web 14, type 3, save 40.33) ties v1 and a0 at cost 0 and keeps v1. Collection temp (web 45, type 4, bbs 6 and 8) takes a0. `recordIndex` takes a1. Target wants record in a0, recordIndex in v1, temp in a1.
- forced packing on this body, scored with `--object`, acceptance `forced=-1`: `p1:w14=c3` plus `p1:w45=c4` is 0 masked at delta 0. Synergistic (pair 0 vs expected 6). That packing is diagnostic only and is not a match.
- L160 attempts that did not unforce the integer pair (size 0 kept unless noted): indexed `record[i]` (delta -4); `record` live across `o13GetView` (delta -4); even/odd second pointer (frame 0x170); L109 OR/XOR/AND on `recordIndex` in the walk or sort (CSE, byte-identical); dead s32 inits of done or temporaryIndex (eliminated, L99); line-join of the three inits (byte-identical); subscript init of `record` folded to `D_0`. Do not retry those.
- next action: a mutate-existing-carrier spelling that makes `record` skip v1 so a0 wins, without a twelfth home and without deleting `record` or inlining dx. Raising `recordIndex` save above 40.33 in the collection loop (uses that are not CSE'd) would colour it onto v1 first and leave a0 for `record`. Force-0 remains not a match.
<!-- plateau-handoff:overlay13DrawActive:end -->
