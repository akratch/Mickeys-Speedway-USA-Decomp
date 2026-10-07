<!-- plateau-handoff:shadowGenerate:start -->
### `shadowGenerate` plateau handoff

- source: `src/main/shadows.c`
- score: 0/510 words, promoted
- frame: 0x138
- relocations: 63
- first mismatch: none
- summary: Matched. 419 at +24 to 0 on 2026-10-07 (lane a-shad) by rewriting the inherited m2c shape; the edits are listed in the last section

Summary before this remeasure: Typed owned trap calls remove default float promotion: 432/+40 to 419/+24; exact frame retained. Remaining type-home and structural residual needs new source evidence.

Summary before this remeasure: Frame exact at 0x138 (unused f32s dropped, homes reordered): 445 to 432 at +40; an s16 still spills to +0xAA where the target keeps type in fp

Summary before this remeasure: Fresh V0 is 520/510 words with 445 differences; frames 0x150/0x138. Both have 63 relocations; 29 sites and identities align. Prior mechanisms closed.

## 2026-10-02 (lane x-shad): frame closed, 445 to 432

The 0x150 frame was five unused f32 locals plus declaration order. A
27-cell product over how many scalars sit above `first` (3 to 5), between
`selected` and the angle arrays (6 to 8) and between the arrays and
`objects` (1 to 3) puts the floor at four, seven and two: frame 0x138 with
`first` +0x124, `selected` +0x120, the arrays +0xEC and +0xDC and `objects`
+0xD0, as in the target. Measured flat or worse on that layout: the
s16 locals widened to s32 (objectType alone, or type and lowAngle
together, moves the frame back off, 445; type or lowAngle alone 432), reading
`object->0x44` at each use instead of the `objectType` carrier (441 at +52).
Left at the head: the target keeps the object in s7, the surface in s6 and
the type in fp, where this candidate has s8, s7 and an s16 home at +0xAA.

#### Authenticated single-precision trap transport, 2026-10-03

- The fresh committed authorization at `6278da171` pins source
  `baeed0592e384ed510d1e1d506449086d8ea7adc` and shard
  `f69a6fcb694c54c4096b39ea8b2fe532fc33aed7`; the assignment gate returns
  `base-only`. This packet changes only the TU-local trap declaration used by
  the two owned calls, not their values, order or surrounding function body.
- Mickey main relocation records 178 and 179 bind the calls at owned offsets
  `+0x384` and `+0x414` through ROM-table identity 1330 to overlay 43,
  `.text+0x324`. The exact C owner extends to `+0xBE4`; its committed source
  is `func_overlay_043_F0000324_188A2F4.c`, pin
  `ba0a836b90de3cee988dca5d7437028d0b8fc17d`. Its return is `s32` and its
  third formal is unused. That unused pointer formal does not establish the
  caller's original declaration; Mickey's actual single-precision argument
  transport independently establishes the required caller width.
- Actual configured stock baseline: 520 candidate versus 510 target words,
  432 raw/masked differences, size delta `+40`, exact frame `0x138`, and
  63 candidate/target static relocations with 29 exact identities. The first
  ranking mismatch is `+0x118`, a branch-displacement difference; the
  workbench normalizes that branch relation and first reports a register
  difference at `+0x134`. These comparator offsets are distinct measurements,
  not interchangeable or evidence of stale ranking metadata.
- Captured complete asm-processor compiler input reproduces `.text`, `.data`,
  `.rodata` and all relocation records. Prepared context compares unchanged
  with itself and the owned function comparison is exact. Full symbol tables
  differ only by asm-processor's absolute prelude guard; that known difference
  is disclosed, not treated as full-symbol fidelity.
- The sole local declaration now reads
  `s32 TrapDanglingJump(void *, s32, f32)`. Both existing `(f32) arg1` arguments
  retain their conversions. Removing implicit double promotion removes exactly
  four instructions across the two calls, as predicted: 516/510 words,
  419 raw/masked differences, size delta `+24`, exact frame `0x138`, first
  ranking mismatch `+0x118` and workbench mismatch `+0x134`. No alias,
  rebinding, shared header, compiler flag or callee changes are introduced.
- All nine other full-TU functions retain identical bytes and per-function
  relocation offset/type/identity lists. The intentional declaration-context
  correction is independently reviewed; context comparison must report it as
  changed rather than pretending the outside-function context is unchanged.
- The assigned promotion mechanism is eliminated. Fresh workbench diagnosis
  remains `structure-mismatch` with mixed structural/register residual and the
  existing type-home question; no new source-attributed allocation mechanism
  was established. No closed declaration/home or colour sweep was repeated.
  The candidate stays `NON_MATCHING` with its assembly fallback and earns zero
  matching credit. Baseline, typed source/object, summaries, captures and
  collateral proof remain ignored under `build/shadow-float/`.
- Validation: configured baseline and typed comparisons, captured-input
  self-context and fidelity, independent other-function byte/relocation checks,
  workbench diagnosis, full ROM verification, documentation, clean-room and
  tooling gates before the owned plateau commit.
#### 2026-10-07 (lane a-shad): matched, 419 at +24 to 0

Read from the target listing and measured as fast_score products ranked by
aligned rows (residual_map --object), never by the positional count, which
read worse on several steps that were right. Baseline: 419 masked at +24,
aligned 284 exact, 123 naming, 25 immediate, 68 really different.

  - info->0x14 is a u16 flag word (the target reads a halfword) and
    object->0x6 an s16; object->0x44 read at each use after camGetMode
    instead of the objectType carrier; angleSource as
    `angleSource = NULL; if (flags & 8) { if (link != NULL) ...}`: a product
    of these alone stayed 392 to 441, size +28 to +48.
  - one loop variable for the model-part loop, the sort passes and the
    final call loop (k; the target keeps all three in s2), angleCount
    starting at 0 and set to 1 under D_80079460 > 0, the material stored
    before angleCount++: 318 at -8, aligned 417 exact, 30 naming. The merged
    loop variable is the big one (it frees fp for type, whose s16 home at
    +0xAA was the spill/reload pairs insertion_pairs reported).
  - with that, angleSource as an if/else over `flags & 8 && link != NULL`:
    205 at -4, structural 10.
  - the model tail reading object->0x50 at each use (the target reloads it
    after each func_800180B4 call), the trap result in j instead of value,
    the angle pointer stored after the three part loads, and the sort bound
    assigned in its guard `(k = angleCount - 1) > 0`: a 24-cell product,
    exact at all four together (16 at delta 0 without the store order and
    the j split).
  - objectType deleted (unchanged); deleting the now-unused i moves the
    frame (29), so it stays declared.

gmake verify passed with the guard removed.
<!-- plateau-handoff:shadowGenerate:end -->
