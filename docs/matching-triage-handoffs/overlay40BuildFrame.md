<!-- plateau-handoff:overlay40BuildFrame:start -->
### `overlay40BuildFrame` plateau handoff

- source: `src/overlays/o040/overlay40BuildFrame.c`
- score: 0/81 words, promoted
- frame: 0xB0
- relocations: 1
- first mismatch: none
- summary: Matched. Plain source: one cursor walking the eight records, declared with the scalars before the array, and every field store on its own line.

#### 2026-09-19, lane w26-o040: store schedule on the home-correct shape

Identity gate: instrumented IDO `.text` matches stock (sha1 981a68df). CDX_PROC 0, 13 p1 decisions, 0 p2. Unforced records read forced=-2; a same-colour force `p1:w10=c5` reads forced=5.

The 75-word residual was the independent `bottom = y + height` node: its height load is initially ready after `sp` and wins as1 on aftercycles 39 over colour packing (33). Assigning `bottom` at rec2.bottom via a comma expression (`bottom = y + height, bottomPlus2 = bottom + 2, bottomPlus2`) keeps the named local (inlining is 3 short) but gives the height load `before=3`, so it is not initially ready. Colour packing then occupies the head: first 16 words match the target except `right`'s register (`v1` vs `t0`). `#line 20` on rec2.right/bottom lets `x + 3` beat the ra save (lineno 24). 75 to 60 at delta 0, frame 0xB0, records still at sp+0x40. Aligned exact 13 to 28, immediate 1 to 0, structural 19 to 14, only-words 17 to 14.

Colour is still closed. Named rec7 pointer grows the frame to 0xB8. `records[7]`, `(&records[7])->`, `(records + 7)->` and generated s16 subscripts canonicalize to stack-relative stores. Field reorder of the early target stores saves s0 and adds 3 words. `if (1)` around bottom costs 3 words. Moving the volatile to rec2.bottom costs a size delta.

Next: the +0x44 tie. After `x + 3`, as1 picks `sw ra` (lineno 24, aftercycles 22) over `addu bottom` (lineno 21, aftercycles 22) even though lower lineno should win. Target emits bottom, bottomPlus2, then ra, then the +0x1c volatile round-trip, then rec2.bottom. Rec7 is still five stack-relative stores; the target writes it through `v0 = sp+0x94`.

#### 2026-10-02, lane x-ovlb: matched and promoted

60 -> 0 at delta 0, frame 0xB0, one call relocation, verified. The earlier
candidate's tools (#line directives, a volatile round trip, an 8-byte
scratch aggregate) are gone. Three facts, measured in this order:

- A cursor walking the array (`rec = records; ...; rec++`), not constant
  subscripts. With subscripts the function was 12 bytes short. With the
  cursor, IDO puts the last record's base in v0 (sp+0x94) and spills one
  shared sum to sp+0x1C for register pressure. Both are target features
  that earlier lanes tried to force by hand. 47 words, delta 0.
- The cursor and the three scalars are declared before `records`. That puts
  the array at sp+0x40 and leaves only as1 scheduling, at 23 words.
- Each field store on its own line. A one-line RECT macro gives a record's
  five stores one line number, and as1's tie-break (L59/L132) then emits
  them in the wrong order. Written longhand, the function is exact.

The resident callee is reached through the `overlay40DrawRectanglesReloc`
placeholder (frontDrawRectangles, SYMBOL record), so the object needs no
rename rule.
<!-- plateau-handoff:overlay40BuildFrame:end -->
