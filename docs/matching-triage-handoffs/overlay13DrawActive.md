<!-- plateau-handoff:overlay13DrawActive:start -->
### `overlay13DrawActive` plateau handoff

- source: `src/overlays/o013/overlay13DrawActive.c`
- score: 38 differing words
- frame: 0x168
- relocations: 10
- first mismatch: +0x40
- summary: Size and 0x168 homes exact. Unforced 38 naming. Triple force scores 0 at delta 0. L160 next.

- geometry: Overlay 13 `+0x874..+0xB0C`, ROM `0x186F38C..0x186F624`, 664 bytes / 166 words both sides, size delta 0. `overlay13Call` follows with no padding.
- ABI/flags: `void overlay13DrawActive(s32, s32, s32)` at Overlay 13 `-O2 -mips2 -32`. Frame `0x168` both. Slot ladder identical: `+0x170 +0x16C +0x168 +0x164 +0x158 +0xDC +0xD8 +0x50 +0x4C +0x1C +0x18`. Save set is `ra` and `s0` only.
- aligned residual: 128 byte-exact, 38 register naming, 0 immediate, 0 structural, displacement tax 0. First naming `+0x40`. Dominant integer cycle `v1 -> a0 -> a1 -> v1` (97 percent coherent); float cycle `f0 -> f2 -> f0`.
- identity gate: instrumented IDO `.text` is byte-identical to stock. `CDX_PROC=0`, 33 p1 decisions. `--every-colour` is legal (size delta 0) but not required: three same-kind forces already pack to 0.
- what closed size and frame (lane w23-o013):
  - L97 `if (1)` around the record walk splits the `D_0` address-constant web, so it rematerializes in the draw loop instead of taking `s3`. That dropped `s1`/`s2`/`s3` and closed the one-word deficit (165 to 166).
  - L99 declaration order `i`, two unused pointers, `count` above `indices`/`distances` puts the jal spill homes at `+0x164` and `+0x158`.
  - L112 `indices[26]` (not 32) pays the remaining 16-byte frame. `distances[35]` is required for bases `+0x4C` / `+0xD8`.
  - Init order `recordIndex = 0` then `count = 0` matches the two zero-moves after `o13GetView`.
- forced packing (scored with `--object`, acceptance `forced=-1`): `p1:w11=c3` (walking `record` onto `a0`) plus `p1:w76=c25` (FP web onto colour 25) plus `p1:w42=c4` (collection temp onto `a1`) is 0 masked at delta 0. Pair `w11+w76` is additive 12; `w11+w42` is synergistic. Identity gate PASS on that TU.
- L160 blocker: the three colours are reachable. Unforced, web 11 (`record`, type 3) ties v1 against a0 at cost 0 and keeps v1 (lower colour number). Deleting `record` (indexed `D_0[i]`) or inlining dx, dz, dy reopens size. Inner-local `record` is byte-identical to function-scope. An L109 OR-with-zero on `recordIndex` in the walk was CSE'd away.
- eliminated on this body: L90 `<` vs `!=` on the walk (flat); draw-loop countdown/pointer walk (flat or worse); `do { } while (0)` vs `if (1)` (flat); JFG `dropletDrawAll` (Jaccard 0.24, no source). Further same-kind forces on the held triple did not beat 0.
- next action: L160. Change the source so globalcolor chooses `record=a0`, the FP web colour 25, and the collection temp `a1` with no `CDX_FORCE`. Do not reopen colour landscapes until an unforced compile is 0 or a new carrier shape invalidates the 0-packing.
<!-- plateau-handoff:overlay13DrawActive:end -->
