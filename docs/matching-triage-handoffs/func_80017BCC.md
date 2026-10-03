<!-- plateau-handoff:func_80017BCC:start -->
### `func_80017BCC` plateau handoff

- source: `src/main/shadows.c`
- score: 0/314 words, promoted
- frame: 0x108
- relocations: 44
- first mismatch: none
- summary: Matched. 217 to 0 on 2026-10-02 (lane z-shad) by rewriting the inherited m2c shape; the eight edits are listed in the last section

Summary before this remeasure: goto loops as do-while 221 to 217; open: zero constant in f20 vs f12 and the s7/fp batch-counter swap (p1 ranking)

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


## 2026-10-02 (lane x-shad): counterpart named, indexed rewrite measured worse

This function is Diddy Kong Racing's `func_8002F440` (public DKR decomp,
`src/tracks.c`; the shadow vertex and triangle emitter), with Mickey's buffer
limits and early `return 0`, a packed texture word per vertex (u in the high half, v in the low)
and the shadow globals read from the query struct. The early returns skip
the count write-back, so Mickey's source holds the three counts in locals,
as the m2c draft does.

Measured: the DKR shape with Mickey locals written as indexed accesses
(`vertices[i]` of each polygon, `D_800C9D48[...]` and `D_800C9F58[...]`
indexed, vertex and triangle records reached through pointers advanced once
per record, indexed `projected[i]`, typed structs, plain for loops): 267
masked at size +12, frame 16 bytes short. `tools/insertion_pairs.py` reads
the target's address-constant words before the polygon loop (+0xF4 to
+0x118) as target-only and the candidate's in-loop constants as
candidate-only, so the cursor shape of the m2c draft is the source shape
here and the indexed form is not a better base than the 217 draft. The open
decision is unchanged (0.0f in f20 in the target, f12 here).

## 2026-10-02 (lane z-shad): matched, 217 to 0

`gmake verify` prints the expected hash with the function compiled from C.
Every step was measured alone on the step before it (masked words at size
delta 0 unless stated):

  - the sine and cosine as plain assignments from the two calls, with no
    address-form home: 217 to 216. The allocator spills the sine across the
    second call by itself; the store lands in the second call's delay slot.
  - counts read before the buffer cursors, vertex cursor before triangle
    cursor: 216 to 212. The tell is which four high halves as1 pulls up
    into the head block; they are the first four global loads in statement
    order.
  - the two scales divide by the query fields and the half extents are
    copied to locals afterwards: 212 to 166 at +4. The shared field loads
    become expression temporaries that hold f0 and f2 in the head block
    (as1 folds their copies away, so nothing shows), which is what denies
    f0 and f2 to the height and the zero constant. Scale webs then tie the
    extent webs on save and win on web number.
  - a float `fade` separate from the centre x: 166 to 155. The zero
    constant takes f20 and the fade f18.
  - `fade *= 1.0f - ...` on a fade initialised 255.0f, instead of
    `fade = 255.0f * (...)`: 155 to 146. uopt propagates the constant into
    the multiply with the constant as left operand, which is the shipped
    operand order and free-list order.
  - in-place rotation with one saved copy (`x -= cx; z -= cz; saved = x;
    x = x * c - z * s; z = z * c + saved * s;`): 146 to 130. The x and z
    webs tie at nine references and x wins on number; as1 deletes the copy
    by renaming, which is the target's separate f12.
  - polygon pointer and index initialised at the loop, the vertex cursor
    not assigned in the loop head, the `u8` count carrier removed: 130 to
    75 at delta 0. The index web then outranks the batch counter (s7
    against fp) and the count is one web in t0.
  - batch stores in the order texture, vertex count, triangle count, and
    vertex stores in the order x, y, z, colour bytes on separate lines:
    75 to 60.
  - `(s16)` on the low texture half only: 60 to 38. The cast is two ring
    draws that as1 deletes; with it on both halves the high half is one
    draw long.
  - `&D_800C9F58[index << 5]` (subscript, base first) and the second
    triangle index initialised from the loop index variable instead of the
    literal 1: 38 to 35; the declaration list without padding
    (`projected[6]`, six scalars above it): 35 to 26.
  - the polygon vertex index read by subscript `polygon[i + 2]` with no
    declared byte cursor: 26 to 4. uopt creates the cursor and as1 pulls
    its copy into the loop head beside the pulled high half.
  - `polygon = D_800CAF60;` on its own line and the index zeroed in the
    `for` header: 4 to 0.

Two things here are general. as1 pulls constant materialisations and
register copies up from later blocks into an earlier block that has idle
cycles, one instruction per trial, and keeps a pull only when the donor
block gets shorter; so where a `lui`, `li` or `move` sits in the target says
little about which block its statement is in, and the order of the pulled
group is statement order. And a field load shared by two statements is an
expression temporary with a colour of its own even when as1 removes its copy.
<!-- plateau-handoff:func_80017BCC:end -->
