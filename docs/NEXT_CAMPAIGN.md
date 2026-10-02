# Next wave after wave X (2026-10-02)

This file is the single next-wave handoff. It replaces the wave-7 handoff.
Every byte total below was recomputed from the integrated tree when this
file was written. Recompute before quoting.

## Where the tree is

```
resolved 804,716 / 943,640 = 85.28%
85% = 802,094 bytes, crossed by 2,622; 90% = 849,276 bytes
queue 84 functions / 127,580 bytes
```

`gmake progress` printed `functions: 1310 matched / 1460 total (89.73%)`.
The integration verify printed `OK build/mickey.us.z64 matches the expected
US ROM hash` (`507341c0a40ca3e9a7cee969b396ee53facfb548`). Wave X started at
741,548 bytes (78.58%), crossed 80% about forty-five minutes in, and
crossed 85% when `overlay1LoadBuildRecords` matched. The overlay 26 update
(2,424 bytes) followed from the candidate lane w2-ovlc left at 36 words.

Every lane worktree is clean and nothing is running. The wave stopped on
account usage, not on the queue.

## What chose the targets

The wave before this one spent seven lanes on small allocator plateaus that
had already had four to six passes and matched nothing. Wave X ranked the
queue differently, and both rules are worth keeping:

- **Has the function had a rewrite pass since the shape checklist existed?**
  Count the dated sections in its shard. About sixty queued functions had
  none, and most of the wave's matches came from them, including several
  2-3 KB functions at 90% residual that had never had a lane.
- **Does a matched function have the same shape?** `tools/sibling_scan.py`
  ranks every unmatched listing against every matched compiled function in
  about two seconds. Re-run it after each batch: new matches become new
  siblings (`fxScreenEffect` matched, then became the lead for an overlay 66
  function an hour later).

Three kinds of code were invisible to the ranking and so to every earlier
lane: a body shared by two overlays through `#include` and macro renames
(overlays 69 and 88), a body kept in an `.inc` file (overlay 14), and a pure
`GLOBAL_ASM` function with no candidate (overlay 51). Compare the progress
report's NON_MATCHING bytes with the ranking's queue bytes; a difference is
code nobody is looking at.

## Near-misses, each with its next step

Numbers are from the ranking at the time of writing.

- `func_8001357C` (src/main/track.c): 1,040 bytes, 2 masked words, size delta 0. The target loads the surfaces base between the plane-index load and its scale, base first in the add. 900 expression spellings scale first; a declared index gives the order but takes a colour. Next is an instrument question: which ugen handler emits an add whose second operand is a shift. The +0x1A8 pair of `func_8001398C` is the same blocker.
- `func_overlay_027_F0000624_187BFFC` (src/overlays/o027/overlay_027.c): 1,016 bytes, 12 masked words, size delta 0. Two colour decisions on the two vertex-address webs (a0 and v1 here, a3 and a1 in the target). Forcing both (`p1:w119=c6`, `p1:w141=c4`, accepted) scores 0. Measured since: a local, a direct symbol, two locals and a cast round-trip all compile identically (uopt makes one address web either way). The records give web 119 cost 0 on a0 through a3 with only v0, v1 and s0 forbidden, so it takes the lowest. The target's a3 needs three higher-priority webs holding a0-a2 in block 15, and the target's argument setup for the next call starts with a3: look for call-argument values that are webs live in that block.
- `func_overlay_044_F0000580_188BDE0` (src/overlays/o044/func_overlay_044_F0000580_188BDE0.c): 1,396 bytes, 13 masked words, size delta 0. The stride conversion temp ties `xh` and `dsdx` on save and loses on web number. Forcing three webs scores 5, schedule-only. Give the stride temp a second reference in the preheader, or number it ahead of `xh` (first-occurrence numbering, checklist item 28).
- `func_8000E5EC` (src/main/track.c): 820 bytes, 12 masked words, size delta 0. Two of four forces are needed. An empty `if (camera) {}` closes the camera pair (9) but is a diagnostic construct. The other pair needs the list cursor's save at exactly 30; block wrappers overshoot.
- `func_80051364` (src/main/anim.c): 1,148 bytes, 9 masked words, size delta 0. A scratch shape reaches the target's a0/a1/a2 with no force at 7 words: the join written cursor increment, subtraction, state store, the first two each in a one-line `do { } while (0)`, and four blocks removed elsewhere (two early returns nested, the `updateRate <= 0` return nested, the camera clear as a do-while) so timeScale keeps f20. Not adopted in the tree. The target computes the state conversion before the cursor add (both hoisted above the float compare) and stores subtraction, cursor, state. A further 205 cells (six statement orders, every region mask, a dead read into pad, state and cursor locals, an empty if) floor at 7: state-first gives the draw order and swaps the cmdWord and clock colours.
- `overlay15InitStarsAndPalette` (src/overlays/o015/overlay_015.c): 988 bytes, 17 masked words, size delta 0. The palette tail is IDO's unroller on a one-variable `for`. A two-variable form restores starIndex's rank at 197/-8 because the unroller copies only the primary variable. Make starIndex an independent induction variable.
- `func_overlay_020_F000038C_1876964` (src/overlays/o020/func_overlay_020_F000038C_1876964.c): 1,080 bytes, 251 masked words, size delta -4. One word short. The target computes all four vertex indices into saved registers before the first division and keeps the texture in fp.
- `overlay69DrawSortedGeometry` (src/overlays/o069/overlay69DrawSortedGeometry.c): 1,436 bytes, 57 masked words, size delta 0. One body shared by overlays 69 and 88 (2,872 B together). The whole residual is one scratch draw before the geometry index load that must emit nothing. Every source construct that spends it is normalised away by uopt or emits a word. This is an instrument task (trace the ugen path on the `*(state->fixedGeometryIndex + i)` cell), not another spelling sweep.

Also moved a long way without closing, with the decision variable recorded
in each shard: `func_80053868` (522 to 393), `func_80009414` (398 at +20 to
243 at 0), `func_8002FB34` (272 at -20 to 155 at 0), `func_8002EBE0` (218 to
89), `func_overlay_058_F00005FC_18AF7E4` (109 to 89), `overlay68UpdateAnimation`
(180 to 108), `func_overlay_002_F0001DF8_1858BF0` (353 to 198),
`func_80016890` (534 at +28 to 439 at +12), `overlay17CreateChain` (83 to 65).

## Not started

- `func_overlay_047_F0000B30_1891948` (8,672 bytes). A dispatch was written
  and never ran. The untested lead: its shard's decision variable 2 says a
  second extern name for the first loop reproduces the target's separate
  address materialisations and was rejected as an alias artefact. Checklist
  item 25 was measured three times this wave: decode the relocation records
  for every site naming that address before believing it is one object.
- `overlay68UpdateAnimation`, `func_8003C80C` (0.44 to the matched
  `overlay34InterpolateColor`), `overlay83DrawStrip`, `overlay48InitializeState`:
  a lane was dispatched and stopped before its first compile.
- `func_8004B1DC` and `func_80001BF4`: the owning lane closed its other two
  targets and stopped.
- `func_8001EC44` and `func_8001E5C4`: `D_800CB2C0` is now carved into
  `charControl.c`'s `.bss`, which is what the target's one-high-half-per-access
  stores need (as1 shares a high half only for symbols the TU defines). The
  product on `func_8001EC44` from the recorded 201 and 221 cells is next.
- `overlay99ApplySegment`, `func_overlay_043_F0000324_188A2F4`,
  `func_overlay_056_F00001A0_18A2F18`, `overlay98RenderReflections`,
  `func_overlay_009_F0000000_1866678`, `overlay100DrawMotion`.

## Closed with evidence; do not reopen without a new lever

- `matrix.c`: the ROM uses odd single-precision float registers no IDO
  build emits. See the file's own notes.
- `func_800517E0`: 332 bytes short on two allocator outcomes; per-case
  locals were measured and did not move it.
- `overlay57UpdateModeState`: listed in `config/unassignable-symbols.us.json`.

## What the tooling learned

- `tools/merge_lanes.sh` re-measures the ranking after merging and runs
  `check-tooling` with the other gates. A batch that stops part-way leaves
  merge commits without the regeneration step, so `verify` and `check-docs`
  read red (a stale `config/overlays.us.json`) until the batch is re-run.
- `mk/overlays.mk` grows by a rule per promotion and crossed the clean-room
  256 KiB limit during this wave. Two finished overlays' rule blocks now
  live in `mk/overlay_014.mk` and `mk/overlay_057.mk`, included at their
  old positions. The file has roughly fifteen promotions of headroom; move
  another finished overlay's contiguous block the same way when it runs
  out, and prove it with a sorted `gmake -pn` comparison, because objects do
  not depend on mk files and `verify` alone proves nothing.
- Gaps lanes reported: `finalize_plateau.py --commit` refuses the
  regenerated ranking that `check-docs` then demands; `shape_product.py`
  omits value 0 for an axis never compared with 0; `fast_score.py` refuses
  a symbol with no `NON_MATCHING` block and its `git grep` fallback finds
  nothing on macOS; `donor_match.py` misses a counterpart that calls only
  unnamed helpers (three DKR counterparts were found by hand).
  `tools/resident_storage_view.py --key gravity` refuses after the
  `charControl.c` carve and was not investigated.

## Next concrete action

One owner per translation unit, as before. In this order:

1. Check every near-miss above for the pattern that closed
   `overlay1LoadBuildRecords` (checklist item 31): a conditional store
   where the target loads a global through a held register and stores it
   with a fresh high half.
2. Promote nothing from the scratch candidates without re-measuring them on
   the current tree: adopt the overlay 26 candidate and the `func_80051364`
   shape, bank each, then work the residual.
3. `func_overlay_027_F0000624_187BFFC` and `func_overlay_044_F0000580_188BDE0`:
   both score zero or near it under accepted forces, so each is a question
   about one ranking, not about shape.
4. Run `tools/sibling_scan.py --min 0.4`, then give the functions under
   "Not started" one rewrite pass each.

A twenty-lane fleet used a week of account usage in about four hours.
Fewer lanes, each resumed on its own recorded next step when it finishes,
produced as much per token as fresh lanes and kept the module knowledge.
