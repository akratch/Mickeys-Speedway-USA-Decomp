<!-- plateau-handoff:overlay10Initialize:start -->
### `overlay10Initialize` plateau handoff

- source: `src/overlays/o010/overlay10Initialize.c`
- score: 0/172 words, promoted
- frame: 0x58
- relocations: 41
- first mismatch: none
- summary: Matched. Every loop is a plain subscript loop, width and height are read directly with no copies, and the per-file unroll override is removed.

Summary before this remeasure: hypothesis=hoist width and height into a1 and a2 without extra s32 homes; spellings=param-pin scored 173, register copy inert at 37, OR-zero deleted reloads and grew 8 bytes; stall=none lowered the masked count at delta 0 without growing the frame

#### 2026-10-01, lane b-o057: ROM-exact closure by writing the loops as subscripts

The 37-word plateau walked declared pointers and stored through index minus
one, copied width and height into two extra locals so that the stores through
the walking pointer could not alias them, read the entry table through a
volatile pointer with an or-with-zero on the offset, and carried a per-file
flag switching the unroller off. Each of those was compensation for the
walking pointers. A store through a subscript on a static array cannot alias
an address-taken local, so width and height hoist without copies; uopt builds
the end-pointer tests itself; and the entry loop reloads its table pointer per
store with no volatile.

Steps, each measured at the default flags:

- inherited shape: 37, frame 0x68
- the stale lane's indexed first loop, read-only reference: 25, frame 0x60
- all four loops as subscript for loops with the last nest indexing the
  resource table too: 178 and 28 bytes long, but the first three loops are
  exact; the excess is extra induction pointers in the last nest
- the last nest with a walking resource pointer, buffers still subscripted:
  4, all in the schedule of the resource pointer's load before the loop
- the resource pointer initialised and stepped in the for header instead of
  on the line above: 0. Four placements of the initialisation on the for
  line all measure 0; do and while spellings with it on its own line are 4.

The accumulator of the entry loop and the inner index of the load loop are
one local, which is why the accumulator takes a saved register in a loop with
no call. The per-file unroll override in the overlay makefile was refuted by
this shape and is removed: the function measures 0 with and without it.

Verified by the ROM hash, the overlay alias drift check and the per-symbol
promotion proof.

Summary before this remeasure: loopunroll,0 closes size. leftover offset OR-zero closed slti vs li. Copies cost 16 frame bytes and buy the first-loop shape.

Configured compile carries -Wo,-loopunroll,0 (NON_MATCHING-only). Identity-gate: instrumented .text matches stock, CDX_PROC=0.

Closed this lane: size 688, delta 0 (was +156). leftover offset OR-zero in the entry loop closed the extra/missing pair (li 4096 vs slti). D_140, D_400, D_10 are the splat-named loop ends.

Blocker: frame 0x68 vs 0x58. `widthValue`/`heightValue` copies are load-bearing for increment-first viewport stores and hoisted width/height. Each copy costs 8 frame bytes and saves about 4 code bytes. Zero copies: frame exact, size +8, the loop reloads width/height every iteration. `register`, nested blocks, an s32[2], and a pre-incremented cursor all failed to hoist without homes.

First mismatch +0x0 is the frame addiu. 29 naming rows: a0/s0/v1 cycle on angle/offset/entries; a1/a2 vs v1/a0 on width/height. Splitting the inner-loop index from `offset` shifted s-regs and scored 44.

Next: a spelling that hoists width/height into a1/a2 without extra s32 homes, then colour. Do not run a colour landscape until size and frame are both 0.
<!-- plateau-handoff:overlay10Initialize:end -->
