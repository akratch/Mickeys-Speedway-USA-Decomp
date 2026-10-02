<!-- plateau-handoff:func_overlay_079_F0000FA0_18CDF40:start -->
### `func_overlay_079_F0000FA0_18CDF40` plateau handoff

- source: `src/overlays/o079/func_overlay_079_F0000FA0_18CDF40.c`
- score: 41/184 words
- frame: 0x98
- relocations: 13
- first mismatch: +0x3C
- summary: Fresh reproof unchanged; no caller, Conker donor, or proxy evidence resolves the 13-to-5 relocation mismatch.

### 2026-10-02, lane w7-o079: sibling plane shape grows the frame

Per-file flags on this object are only `-Wab,-r4300_mul`. No `-Wo,-loopunroll,0`, `-Olimit`, or `-O2 -g3` is present, so nothing was removed. The multiply-hazard flag stayed.

`tools/shape_lint.py` reports three artefacts: the four `f32` externs, the `volatile` plane constant, and the masked flag test. `tools/overlay_tables.py --json` gives overlay 79 text `0x14E0` and data `0x60` at ROM `0x18CCFA0`. The same module's relocation decoder, filtered to this function (`0xFA0`..`0x1280`), shows 13 records, not 5. Five are `R_MIPS_26` `SYMBOL` calls: `sqrtf` at `+0x10A0`, `+0x11B8` and `+0x1204`, `Arctanf` at `+0x11C4`, `func_8002A8BC` at `+0x11D4`. The other eight are `LOCAL` `HI16`/`LO16` pairs, every one with base `0x1500` and lo addends `0x30`, `0x34`, `0x38`, `0x3C`. Those are `gOverlay79Constants` indices 12 through 15 (`0.707f`, `0.1f`, `0.01f`, `0.01f`), the tail of the pool defined in the neighbouring TU. The old 13-to-5 note counted calls only. The two `0.01f` words are distinct addresses, so one spelling would merge them.

Matched overlay 79 siblings (`overlay79FindNearby`, `func_overlay_079_F0000000_18CCFA0`) are straight field reads and inline literals, not this callback. The same `func_80010900` callback that is matched is `func_overlay_026_F0000B18_187AF10`: initialized normals, one reused length, a plain constant, and a truthy `flags & 0x10000000` test. Its else copies the hit point. This target's else is the arctan slide, so that part was not copied.

One `tools/shape_product.py --jobs 2` on that difference, 144 cells over cross spelling, divide order, equal-versus-not-equal, flag form (truthy, `!= 0`, bitfield), constant binding (four externs, the shared array, two literal spellings), and volatile versus plain. Floor 177 masked words at size delta +12. No cell at delta 0. Inside that shape the cross, divide, compare, flag, and volatile axes were flat; the array binding tied the floor. Not adopted. The kept body remains the carrier form.

Follow-up 1. Splitting `projectedX` so `crossY * nz` is its own statement before `crossZ` was byte-identical: 143 masked, size delta 0, first masked mismatch `+0x3C`. uopt folds the split. Eliminated.

Follow-up 2. Dropping `volatile` on the plane constant stays at size delta 0 but scores 144 masked, one word worse. Restored.

Kept measurement: 736 bytes, 143 masked of 184 words, 144 raw, size delta 0, frame `0x98`, first masked mismatch `+0x3C` (raw `+0x38`). Aligner: 65 byte-exact, 80 register naming, 5 immediate only, 38 really different, displacement tax 20. First naming `+0x3C`, first immediate `+0x60`, first structural `+0x9C`. Register census: no integer substitutions. Float swaps `f8`/`f6` and `f4`/`f10` dominate, but coherence is 73 percent across 16 windows, so it is not one ring phase. Frame census: both frames `0x98`. Target-only homes `0x84`, `0x74`, `0x70`, `0x6C`, `0x68`, `0x54`. Candidate-only homes `0x80`, `0x7C`, `0x78`, `0x60`, `0x50`. That is not a one-pad shift. The sibling local set moved the frame by 12 bytes instead of landing those homes.

Tool gaps: `overlay_tables.py --json` prints module headers only; per-site op and addend come from `read_module_relocations` in that script, which has no overlay filter. `shape_product.py` only treats `==` and `!=` as axis values. No full ROM verify, because the result is not exact. A cold `gmake -j2` failed at link until `gmake overlay-syms`; the retry linked.
<!-- plateau-handoff:func_overlay_079_F0000FA0_18CDF40:end -->
