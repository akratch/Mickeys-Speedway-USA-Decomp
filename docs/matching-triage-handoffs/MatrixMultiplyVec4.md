<!-- plateau-handoff:MatrixMultiplyVec4:start -->
### `MatrixMultiplyVec4` plateau handoff

- source: `src/main/matrix.c`
- score: 47 differing words
- frame: frameless
- relocations: 0
- first mismatch: +0x0
- summary: Configured -O2 is 53 of 53 words and all 47 differences are floating-point register mismatches. The first divergence is the odd-single class. No supported source lever; reopen only for reservation-aware codegen.

The configured full-TU baseline is 53 candidate words and 53 target words,
212 bytes, size delta 0, frameless, and 0 relocations. 47 words differ, and
all 47 are floating-point register mismatches. Six words match. The first
mismatch is +0x0.

The workbench's first divergence is the floating-point register class, at
the first pool slot and temporary slot 4. Alignment inserts gaps, so the
structural row count is not the measure. The supported-lever result is
none. There is no constant mismatch. Stock `-O2 -mips2` emits no odd
single-precision registers; the target uses them. `docs/modules.md` section
6.2 already closed flag and IDO-version sweeps for this reason. No new
source spelling was tried. Reopen only for a code generator whose odd
register free list follows the even-path reservation logic, or for a
matched non-assembly donor that uses odd singles with a working recipe.
<!-- plateau-handoff:MatrixMultiplyVec4:end -->
