# Next wave after the seven plateaus (2026-10-02)

This file is the single next-wave handoff. It replaces the overlay 1
promotion brief. The byte totals below were recomputed from the
integrated tree after this wave's seven lanes were merged. Recompute
again before quoting them. No function in this wave is an exact match.

## This wave

Zero exact matches. One adopted improvement, still short of the ROM.
Six plateaus with the source left as it was.

**Adopted, not a match.** `overlay1LoadBuildRecords`
(`src/overlays/o001/overlay_001_head.c`, 2,288 target bytes). The clear
loop now writes each field through `D_1BA0[index]` instead of walking
one pointer backward. That moved the size deficit from 92 bytes to 52
(13 words short) and the masked residual from 470 words to 469. Frame
stays `0xD8`, first mismatch stays `+0x34`, relocations stay 114.
Folding `0x1BA0` through `0x1DCC` into one owner went the other way
(549 masked, 156 bytes short) and was reverted. A null-base spelling of
the `0x94` length is byte-identical to `D_1D8C * 0x94`, so it does not
defeat the hoisted multiply by 148. The TU's only flag is
`-Wab,-r4300_mul`. The adopted clear is four source lines shorter than
the pointer walk; four comment lines sit in front of
`overlay1InitializeModeState` so the matched functions below stay on
their original line numbers. `insertion_pairs` was run on the pre-edit
body only.

**Plateaus (ADR 0018: three attempts, no better residual, nothing
adopted).**

- `func_overlay_001_F0001D78_184E158` stays 268 masked of 627 words,
  size delta 0, first `+0x2C`. The matched rank function's typed
  `D_1DA0_State` alias is byte-identical. Checklist item 22 at the
  lap-compare zero stays at 268: that zero is already in the following
  call's delay slot, which is where the target has it. Per-region
  pointer locals shrink the function by 140 bytes, grow the frame from
  `0x70` to `0x78`, and replace the commoned address with a held
  pointer. Forcing the value web onto `a1` is declined. The forbidden
  mask is `v0` through `a3`, and the first legal colour is `t0`. The
  only flag is `-Wab,-r4300_mul`.
- `overlay101BuildPresentationD` stays 130 masked, 824 bytes, size
  delta 0, first `+0x10`. The target stores `0xFF` once and reuses it.
  There is no `0xC0` immediate. `/ 2.0f` and `* 0.5f` compile to one
  object wherever both were built, including the cells that fold to
  `0xC0`. That `0xC0` spelling is the wrong value and grows the
  function. The matched tail's `0.0f` / `0.00f` / `(f32)(s32)1`
  spellings do not split a pool entry here. No per-file codegen flag
  was present.
- `overlay83DrawStrip` stays 69 of 77 words, size delta 0, first
  `+0x4`, frameless. The saved display-list pointer is copy-propagated
  and inert. Writing the vertex length as shifts, or inlining
  `count * 2 + 2`, stops the `doubled * 10 + 28` fold and scores 74 at
  size delta -4. Pointer or-zero on that short shape grows. The opening
  move still needs the display-list parameter off `a0` without a word
  the 304-byte shape cannot afford.
- `overlay48InitializeState` stays 47 masked, 228 target bytes, size
  delta -16, first `+0x0`. A pre-loop index, a cursor induction, and a
  for-init counter all lose the scaled seed cursor. The floors were 60
  and 57 masked at size delta +16. The guarded body is unchanged. No
  colour sweep: the size delta is not 0.
- `func_8003C80C` (`weather_tail.c`) stays 103 of 118, size delta 0,
  first `+0x14`, frame `0x38`. Indexing the destination instead of
  walking the pointer scores 110 at delta +4. `while (var_t0--)` scores
  120 at delta +24. The `for` countdown unrolls to 161 at delta +240.
  The `do-while` at 8 is the size-matching form. The colour loop lerps
  three bytes and copies the fourth, so the rain `0xFF` shape is not
  this function. The only call is after the loop and its delay is a
  spill, so checklist item 22 has no for-init to fill. No per-file
  flags.
- `func_overlay_079_F0000FA0_18CDF40` stays 143 masked of 184, size
  delta 0, frame `0x98`, first masked mismatch `+0x3C`. The matched
  overlay 26 plane callback is the wrong else: this target slides with
  arctan, and that sibling copies the hit point. A 144-cell product of
  the difference floored at 177 masked, size delta +12, with no cell at
  delta 0. Splitting one product is byte-identical. Dropping `volatile`
  on the plane constant scores 144, one word worse. The two `0.01f`
  pool slots are distinct addresses. The relocation count is 13, not
  the old call-only count of 5: three `sqrtf`, `Arctanf`,
  `func_8002A8BC`, and four local pairs. The only flag is
  `-Wab,-r4300_mul`.

## The arithmetic

```
resolved 741,548 / 943,640 = 78.58%
80% = 754,912 bytes, gap 13,364
queue 134 functions / 186,904 bytes
```

`gmake progress` printed `functions: 1291 matched / 1460 total (88.42%)`
and `resolved: 741548 / 943640 whole-program text (78.58%)`. The ranking
snapshot printed 134 queued identities and 186,904 bytes. The gap is
`754,912 - 741,548`. The matched-byte total is the same as before this
wave: every attempt is still `NON_MATCHING`. The integration verify
printed `OK build/mickey.us.z64 matches the expected US ROM hash`
(`507341c0a40ca3e9a7cee969b396ee53facfb548`).

## Tool gaps worth recording

None of these blocked a proof. They are why a lane's first printed row
disagreed with the score it was actually judging.

- `tools/fast_score.py --diff` skips `lui` and `jal` rows, so the first
  printed line can be later than the masked first mismatch.
  `fast_score` also does not print the frame or the relocation count.
- `tools/shape_lint.py` misses a typed declaration and an empty `if`.
  Its extractor starts at a forward declaration, so the first hits in
  `weather_tail.c` belong to the matched `func_8003C770`.
- `tools/shape_product.py` ignores a bare `#else`, treats only `==` and
  `!=` as axis values, reports the floor without naming which cell
  produced it, and does not print an opcode census. One spelling in the
  overlay 48 product never became a value and was not scored.
- `tools/overlay_tables.py --json` prints module headers. Per-site
  records come from `read_module_relocations`, which has no overlay
  filter.
- `tools/insertion_pairs.py` attributes a one-sided word to the nearest
  source line, which does not prove that expression owns the
  instruction. Its ownership check fails when the candidate object sits
  outside the translation unit. It was not re-run after the
  load-records adoption.
- Stock `uopt -Wo,-zdbug:2` aborts in `wrapper_ecvt` and does not print
  a forbidden mask. The instrumented compiler printed the `v0`–`a3`
  mask when `CDX_LOG`, `CDX_OUT`, and `CDX_PROC` were set.

A cold `gmake -j2` in a fresh lane failed at link until
`gmake overlay-syms`. That trap is already in `CLAUDE.md`.

## Next concrete action

Do not reopen this wave's six untouched plateaus, the parked allocator
set, R8, `func_80028FCC`, or either whale, unless the assignment states
a lever the shard does not already record. One owner per translation
unit. Before any other edit on a new target, remove that TU's inherited
per-file overrides (`-Wo,-loopunroll,0`, `-Olimit`, `-O2 -g3`) and
re-score. Leave `-Wab,-r4300_mul` where a matched sibling in the same
TU already needs it. If the strip unmatches a sibling, restore it.

`func_overlay_101_F000C6E8_18E7F08` and both `NON_MATCHING` functions in
`font.c` were not assigned. `lane/p65c-o101b` and `lane/p65g-font` each
have an unmerged plateau commit on those translation units. The DKR
`render_text_string` shape is already the disclosed organisation of
`func_8004B1DC`; the open stall there is the packet cursor. Presentation
A, B, and C stay closed: the shared-shape fold was measured in lane
`s1-trio`, and this wave's colour-pool pass on D says the `0xC0`
expression is the wrong value for D.

Then assign, only after `tools/ready_queue.py` and a zero-exit
`tools/lane_status.py --symbol` `base-only` verdict:

1. `overlay1LoadBuildRecords`, different lever from the one just spent.
   The kept body is the per-field clear at 469 masked and 52 bytes
   short. `insertion_pairs` on that adopted body, then one construct
   that emits the missing scaled cursor or the unhoisted `0x94` length
   without a new relocation and without changing the file's line count.
   Do not repeat the single `0x1BA0..0x1DCC` owner or the null-base
   length. No colour sweep while the size delta is nonzero.
2. `func_overlay_001_F0001D78_184E158`, only with a kill that splits the
   loaded-value web so `a1` is legal, while the commoned address and
   the existing reloads stay. Do not repeat the typed alias, checklist
   item 22 at this zero, or a per-region held pointer.
3. A fresh base-only function from `tools/ready_queue.py`, one per
   translation unit. Checklist item 22 only where a free assignment
   follows a call and is not already the delay-slot instruction.

`track.c`, `anim.c`, `weather.c`, `textures_354C8.c`, `overlay_008.c`,
`overlay_015.c`, and the overlay 58 TU from the previous wave are still
free for a different function only.
