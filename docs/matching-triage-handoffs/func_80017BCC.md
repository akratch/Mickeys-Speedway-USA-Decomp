<!-- plateau-handoff:func_80017BCC:start -->
### `func_80017BCC` plateau handoff

- source: `src/main/shadows.c`
- score: 217/314 words
- frame: 0x108
- relocations: 44
- first mismatch: +0x58
- summary: goto loops as do-while 221 to 217; open: zero constant in f20 vs f12 and the s7/fp batch-counter swap (p1 ranking)

Summary before this remeasure: Frame exact at 0x108; re-measured under the corrected R4300 multiply scheduler, which adds the three FP hazard nops the target carries.

## 2026-10-01 (lane d-res1): goto loops as loops, 221 to 217

The m2c draft's three `goto loop_N` back-edges are written as `do { } while`
loops (polygon loop, vertex loop, triangle loop) with every statement kept in
place: 217 masked at delta 0, first mismatch still +0x58. Aligned residual
before the change: 155 byte-exact, 111 naming, 9 immediate, 43 really
different; one global mapping explains 90% of the naming sites in four
windows (v1, a1, a3 cycle; s7 and fp swap; t6/t8 and t7/t9; f12 to f20,
f6 to f4, f8 to f6).

Measured flat: splitting the triangle loop's `var_v0` and `var_v1` into their
own locals (using the two pad slots, so the frame is unchanged), all four
cells 217.

What the record says about the two coloured pairs: the target keeps the 0.0f
constant in f20 (rematerialised in each of the three arms of the head) and
temp_f12, the sine and the cosine in f12, f14 and f16; here the zero takes f12
and temp_f12 f0, so the zero web outranks temp_f12 here and not in the target.
The batch counter takes s7 here and fp in the target against the polygon
counter, the same kind of save-ratio swap. Neither was moved by a source edit
tried so far.

## 2026-10-02 (lane e-res3): no change, 217 held

Loop forms as a 8-cell product (polygon, vertex and triangle loops as do-while
against for): every for form is worse (259-277, sizes +4 to +24; the polygon
loop as for is size -4 at 263). Store order of the vertex, triangle and
triangle-word groups (reversed, 8 cells): 217 or 218. A float `zero` local
shared by the head (partial form) costs +12 bytes. The target keeps 0.0f in
f20 across the three head arms (`mtc1 zero,f20` is redefined in each arm after
the calls); here it is a caller-saved f12 web per use, which is the open
decision variable.

<!-- plateau-handoff:func_80017BCC:end -->
