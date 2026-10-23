# Matrix boundaries for future native work

This reference describes `mtxf_mul`, `func_8002AA50` and their immediate format
and trig dependencies. The source references below were checked against
`0d2c6336dea35316ec792e9c781991c1adab6195`; the audit used Mickey's own ROM-exact
linked providers. No assembly was replaced and no new matching credit is claimed.
See the [boundary map](native-boundaries.md) for the wider native-start scope.

## Float multiplication and aliasing

`mtxf_mul(lhs, rhs, dest)` takes three pointers to contiguous 64-byte float
matrices: sixteen four-byte cells, row stride 16, target big-endian storage.
It computes the row-major product `lhs * rhs`. The neighboring point transform
establishes row-vector use, with translation in the final row.

Each product is a single-precision multiply. For one output cell, let `pk` be
`lhs[row][k] * rhs[k][column]`; the accumulation is
`(p1 + p2) + (p0 + p3)`. Fused operations or reassociation do not preserve this
contract automatically. FP rounding, exception and denormal state remain
ambient inputs; this function does not configure them.

The reported nonargument read is preservation: entry saves f2 with an eight-byte
store, arithmetic overwrites f2 from lhs before using it, and exit restores it.
There is no hidden incoming matrix operand. In the observed main-thread FR1
mode this preserves the full f2 register; f3 is untouched by this body.

| Effect | Established contract |
|---|---|
| Reads | `lhs[0,64)`, `rhs[0,64)`; rhs is reread for each output row. |
| Writes | All sixteen destination cells, `dest[0,64)`; no global scratch. |
| Alignment/stack | Matrix words require four-byte alignment; its private eight-byte save requires an aligned stack. No capacity/null checks. |
| Registers | a0/a2 advance by 64; t0 reaches zero; a1 is unchanged; no scalar result in v0 is established. FP scratch is f4/f6/f8/f10/f12/f14/f16/f18; f2 is restored. |
| Lifetime | Sources remain stable until their last reads; destination is writable for the call. No pointer is retained. |

All four coefficients of an lhs row are loaded before its first output store.
Consequently:

- Disjoint matrices work.
- `dest == lhs` works when rhs is disjoint. Later lhs rows are still intact.
- `lhs == rhs` works when dest is disjoint.
- `dest == rhs`, including all three equal, is not generally supported: writes
  overwrite rhs values needed by later rows. Arbitrary partial overlap is not
  an established API contract.

The actual literal left-input aliases occur in
[overlay 43](../src/overlays/o043/overlay_043.c#L327) and
[overlay 98](../src/overlays/o098/overlay_098.c#L229). Current camera calls use
distinct named matrix buffers; the earlier camera citation was incorrect.

## Transform construction and the private trig contract

`func_8002AA50` takes a transform pointer in a0 and a writable float-matrix
pointer in a1, with no defined scalar result. It reads signed angle halfwords
at +0/+2/+4, float scale at +8 and translation words at +12/+16/+20. It does
not read +6/+7. The readable input intervals are `[0,6)` and `[8,24)`;
the writable output interval is `[0,64)`, with four-byte alignment.
[CameraScaledTransform](../src/main/camera.c) and
[ModelRenderTransform](../src/main/models_5B300.c) expose this layout.

The output has a uniformly scaled 3×3 rotation block, zero in the first three
cells of its last column, translation in the last row and one at [3][3].
Translation is copied as words, preserving its bits. All used input fields are
captured before the first output store. Hardware-level overlap can therefore
work with sufficient backing; a 24-byte C transform alone is insufficient
backing for a 64-byte destination. No literal overlapping caller was found.

With other rotations zero, a positive quarter-turn in the first field sends
+X toward −Z, the second sends +Y toward +Z, and the third sends +X toward +Y
under this row-vector convention. Camera calls these y/x/z rotation. This does
not establish every caller's axis naming or global scene handedness.

For each angle the routine calls `func_8002A8C0` then `func_8002A8BC`.
C0 is sine-phase lookup; BC adds a quarter-turn and falls into it. These are
table-based approximations, not permission to substitute host libm. Their
internal requirements go beyond their C parameter/result declarations:

| Requirement | Reason |
|---|---|
| C0 preserves a0 | The following BC call needs the same angle. |
| Both preserve a1/a2 | Destination/source pointers remain live across all six calls. |
| Both preserve accumulated FP temporaries | The six call sites retain respectively none, f2, f2–f3, f2–f4, f2–f5 and f2–f6. Current trig bodies change only f0 among those FP registers. |
| Both avoid stack writes | The caller allocates eight bytes, with saved ra at sp+4; it has no ordinary sixteen-byte argument-home area. |

The existing leaf trig bodies satisfy those requirements. An otherwise ordinary
ABI-compatible replacement need not. AA50 itself uses a0/a2, at/t0–t2 and
f0–f19 as temporaries, preserves ra/sp and leaves saved GPRs and f20–f31 alone.
Every numerical temporary it uses is defined internally.

FP mode is a separate resource contract: [boot.c](../src/main/boot.c) creates
mainThread with positive ID 3, and
[osCreateThread](../src/libultra/createthread.c) sets Status.FR, clearing it for
negative IDs such as audio's −4. Exception/context code restores that status.
This establishes the observed main-thread FR1 setting, not a complete OS audit
or proof that every possible caller runs on that thread. Compiler pairing
conventions must not be substituted for actual hardware register mode.

## Formats, lifetime and remaining source work

Both routines above produce floats. The separate `mtxf_to_mtx` boundary
multiplies by 65536, converts using current FP rounding and writes upper
halfwords into the first 32 bytes and lower halfwords into the next 32.
The [SDK Mtx type](../include/n_audio/gbi.h) describes this packed s15.16 format
and requires eight-byte alignment. Equal 64-byte extents do not make float and
fixed matrices interchangeable. Nonfinite/out-of-range conversion remains open.

Render callers convert CPU float scratch into separate fixed matrix storage,
emit its address and advance the fixed cursor by one 64-byte record. The fixed
storage must outlive its graphics consumer; float locals can end with the CPU
call chain. Allocator capacity, frame completion/free and concurrent camera
scratch use are not settled by this audit.

Two concrete source packets remain:

- [func_8005B644](../src/main/models_5B300.c) passes `(u8 *)temp - 8` as a matrix.
  The stock object's actual 64-byte matrix occupies stack `[0x40,0x80)` in an
  `0x88`-byte frame, but the declared temp starts at +0x48. Recover that genuine
  storage view with exact object/identity/ROM proof; the source's pointer before
  its object is not an additional matrix input contract.
- Overlay 56 uses an f32-shaped type for both float scratch and packed output.
  Reconcile the value-domain views separately from this provider audit.

These source debts and remaining graphics lifetime questions do not reopen the
verified assembly's matching status. Future translation must preserve arithmetic
grouping, alias limits, input capture, FP environment and the private trig calls.
