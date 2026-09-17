<!-- plateau-handoff:func_80028FCC:start -->
### `func_80028FCC` plateau handoff

- source: `src/main/main.c`
- score: 10 differing words
- frame: 0x18
- relocations: 3
- first mismatch: +0x1C
- summary: Or-chain carrier is allocator proc 40 web 2: v0 not offered (forced=-2). Accepted recolours stay 25. L145/L144 miss the ring-temp copies. Best still 10.

#### Owned pass, 2026-09-10 (lane/o7-small): the shape is known, not a donor

Still ten, and the previous "next lever is a later permitted donor C body" is
superseded. The target's shape is now read out of ugen and named.

The target emits, once per arm:

a normalise of the call result into a RING temp `tN`, a short-circuit branch
on `tN` straight to the epilogue, and a copy of `tN` into `v0` in that
branch's delay slot -- with `tN` running `t6, t7, t8` across the three arms.
That is exactly cfe's short-circuit-or value expansion (L51) -- `v = a;
if (v) goto L; v = b; ...` -- with two properties the retained candidate
cannot reproduce together:

1. the carrier is the **return register**, so each arm pays its own `move v0`,
   and the branch target is the epilogue rather than a join above it; and
2. the operand is a **ring temp**, not a coloured web.

Written as a bare short-circuit-or of the three calls, the same three-node
expansion appears in `cc -S` -- normalise into a temp, copy the temp to the
carrier, branch on the temp -- but uopt colours the operand `v1` and the
carrier `a0`, the two coalesce into one register, and the result is 25
instructions with a single `move v0,a0` at the join. Written as three `if
(ret) return ret;` statements it is 29: an inverted branch plus a `b` to the
epilogue per arm. The target is 27, exactly between them, and no cell of the
lattice below lands there.

**Named decision variable, with the record.** The carrier web's colour. v0 is
not *forbidden* to it -- it is not offered, because the web's span reaches a
call result (L101). Two probes in this TU demonstrate the gate directly rather
than by inference:

- replacing only the first operand with `arg0 != 0`, so that one operand is
  not a call result: it is normalised straight into `$2`.
- replacing all three operands with tests on `arg0`, so the function makes no
  call at all: the operands take `$2` twice.

Change only the first operand back to a call and `$2` disappears from the
assignment. All three of this function's operands are call results, so no
spelling of *this* function can put the carrier on v0.

Exhausted this pass, all at 25 or 29 instructions, never 27: a 1,440-cell cross
product of six operand spellings (a bare call, `!= 0`, `!!`, a doubled `!= 0`,
`> 0`, and a `(u32)` cast before the test) crossed with four groupings, four
declaration paddings, five result forms (returned directly, through an `s32` or
`u32` local, normalised again inside or outside) and three dead-expression
reservation webs; plus, singly,
the written-out value expansion with one, two and three named carriers, `goto`
and `do {} while (0)` and `while (1)` exits, an assignment inside the `if`, a
conditional-expression chain, a bitwise-or chain, and nine carrier types
(`s32`, `u32`, `s16`, `u8`, `u16`, `s8`, `int`, `unsigned int`, `long`).

**Next lever.** Either an instrumented `p1cost` capture confirming v0's absence
from this carrier's candidate list -- which would close the symbol as
unreachable by any source edit -- or a construct that gives the carrier a
definition not derived from a call result while keeping all three `jal`s and
their argument setup byte-exact. The donor line is closed: JFG's
`mainAnyoneHas` is still GLOBAL_ASM at efd5abb and supplies no C body.
#### 2026-09-12 (lane `lane/p7-res2`): the or-chain DOES reach the target's control flow; the 2026-09-10 closure is wrong about that

The retained early-return body still measures 10. The value of this pass is a
falsification, and it is worth more than the three words it did not buy.

**The 2026-09-10 note says every or-chain, goto, do-while-break, nested-if and
early-return spelling collapses to the same 25-instruction a0 form. It does
not.**
That pass varied the operand spelling as one axis, applying the same spelling
to all three operands: six spellings crossed with groupings, paddings, result
forms and reservation webs, 1,440 cells. Varying the three operands
*independently* -- 216 cells over six spellings -- separates them. Sixteen
mixed forms measure **27 instructions, size delta 0, and 14 masked words**, and
their control flow is the target's: three blocks, each normalising the call
result, branching on it straight to the epilogue, and the branch not falling
through to a join above it.

The split is entirely predicted by the operand's *instruction count*. The 64
forms whose three operands all normalise in one instruction (`!= 0`, `!!`,
`(u32) != 0`, `0 !=`) are one object at 25 instructions and 19 masked, exactly
as the closure describes. The 16 forms whose first two operands normalise in
**two** instructions (`!(x == 0)`, or `x > 0U`, which IDO lowers the same way)
are 27 and 14. So the two-instruction normalisation is what keeps uopt from
commoning the three normalised results into one web, and one web is the whole
of the old 25-instruction form.

**What is left after that is the carrier, and it is the one thing the old note
named correctly.** In the 14-word forms the chain's value still lands in a0 and
is copied to v0 once, in the epilogue, where the target keeps each arm's value
in a ring temporary and copies *that* into v0 in the branch's own delay slot.
The candidate spends its extra instruction on the second half of the
normalisation and the target spends its on the per-arm copy; the counts agree at
27 either way. So the decision variable is unchanged -- v0 for the chain's web,
which L101 withholds because every operand's definition reads a call result --
but the surrounding structure is no longer in the way, and a lane resuming here
inherits a body whose branches and blocks already agree.

Two readings follow that the old note forecloses and should not:

1. **A structurally exact base exists at 14**, and it is a better place to test
   a carrier lever than the 10-word early-return body, whose eight
   really-different words are the arms themselves.
2. **The "six operand spellings" axis was never exhausted**, because it was
   swept as a single uniform choice. Any lattice on this function that treats
   the three operands as one axis is measuring 6 cells where there are 216.

Axes covered this pass: the full 216-cell independent-operand cross product over
six normalisation spellings in the plain or-chain. Not covered: the same cross
product crossed with the groupings, result forms and reservation webs of the
2026-09-10 lattice, and an instrumented `p1cost` capture confirming v0's absence
from the 14-word form's carrier candidate list -- which is now a different web
from the one that capture would have found in the 25-word form.
#### 2026-09-12, lane `p23-lastmile4`: exhaustive incumbent colour control

The retained early-return base remains 108 bytes, 27 words, delta 0, and 10
masked words. Its aligned split is two naming rows and eight structural rows
in one window. All 53 legal single-colour probes over seven webs retained the
same score, leaving an empty winners list; no web controls this residual on the
incumbent body.

The named source question from the 14-word structurally exact alternate body
therefore remains separate from this result: find a call-result carrier form
that can reach the target’s per-arm ring temporaries without disturbing the
27-word control-flow shape.
#### 2026-09-13, lane e1: count the normalization draws before reading the return carrier

Fresh configured baseline: 108 bytes, 27 words, delta zero, 10 raw and masked
differences. Alignment: 17 exact, 2 naming, zero immediate, 8 structural,
with no unmatched offsets. First +0x1C, frame 0x18, three call relocations.
Stock and draw-traced full-TU text agree. The retained body is unchanged.
No colour sweep or force was run.

The named Ucode stream has 68 procedures and UGEN emits exactly 68 contiguous
procedure markers. Their authenticated join identifies this function as UGEN
procedure 42; its source-line events independently name the owned body. The
allocator-only mapping command refuses because there are only 66 globalcolor
invocations. That refusal is retained, not treated as a successful allocator
receipt. The Ucode reader and parse_ugen_procedure_index from
allocator_trace_receipt.py authenticate the UGEN join directly; each changed
source is captured and rejoined independently. UOPT and UGEN ordinals must not
be assumed interchangeable on this TU.

The baseline has **one scratch draw**, at the third call's normalization.
The target needs three successive normalization temporaries. The first two
baseline arms branch directly on the raw result and return a constant; there
is no missing folded draw hiding in their traces.

Measured controls, with sources, objects, profiles and aligned deltas retained:

- The mixed OR alternate reproduces 14 masked words at delta zero, but has
  **zero draws**. Its aligned result is 15 exact, 5 naming, 6 structural,
  with a candidate-only offset +0x60 and target-only offset +0x58. The older
  phrase structurally exact describes its branch topology, not byte alignment.
- The ordinary OR form has zero draws, 19 positional differences and delta
  minus 8. Its aligned result is 15 exact, 5 naming and 4 structural, plus
  one candidate-only and three target-only offsets.
- A bounded O1 compiler-stage diagnostic bypasses uopt, yet still draws zero
  scratch registers and emits 112 bytes. It is not a configured candidate;
  canonical flags are unchanged. Its result does not support removing uopt
  as a route to the target's three draws.
- Returning a live normalized local from each early arm gives one draw and
  delta plus 16; separate block-local results give one draw and delta plus 8.
  Neither creates the two missing scratch normalizations.
- Narrow each Boolean operand before OR: six draws, delta plus 16. The
  conversions survive, and each operand is normalized twice.
- Apply a zero-valued integer identity to each already normalized operand:
  **three ordered draws**, but 18 positional differences and delta plus 8.
  Alignment: 15 exact, 8 naming, 4 structural, plus candidate-only offsets
  +0x54 and +0x68. The first two scratch results are subsequently renamed onto
  the shared return carrier, while duplicate normalizations remain. Correct
  draw count alone does not supply the target's live return-value shape.
- Move that identity onto the raw call results: all three draws disappear
  again and the object returns to the ordinary OR result. Thus these draws
  belong to the second normalization, not a free identity operation.

ADR 0018 stall: right-grouping the three-draw expression, making its zero
literal signed, and changing OR-with-zero to XOR-with-zero each produce the
same full-TU text, every per-line draw/emission count, and draw order as the
three-draw diagnostic. These last three controls open no new schedule handle.

The next requirement has two parts: one normalization scratch per arm without
normalizing twice, and a return-carrier lifetime that does not cause as1 to
rename the first two results. The earlier carrier question survives, now with
a measured distinction between UGEN draws and surviving machine registers.
The ten-word early-return body remains the best admitted source.

Evidence is ignored under build/e1/r28 and build/e1/map-main. Commands include
draw_census.py, residual_map.py --object and --against, align_symbol.py,
frame_census.py, score_symbol.py, configured stock compiler, named Ucode and
UGEN index authentication, and finalize_plateau.py. Full-ROM gates validate
the guarded tree, not an exact C promotion of this function.
#### 2026-09-17, lane w6-main: Trap 20 force reopen plus L145/L144

Identity gate: instrumented IDO function text is byte-identical to stock
under the configured command (compiler binary swapped only). Logs went to
CDX_OUT. A non-numeric CDX_PROC printed 66 allocator procedures (0 through
65). This function is allocator proc 40: two earlier TU members exceed
Olimit and skip globalcolor, so UGEN 42 maps to CDX 40. The incumbent
early-return body has two allocator decisions, both argument-home webs.
The or-chain body has four, including carrier web 2 coloured a0.

L101 on that carrier, now with Trap 20-correct records. Its p1cost list
starts at a0; v0 and v1 are absent. CDX_PROC=40 CDX_FORCE=p1:w2=c1 records
forced=-2 and a byte-identical 25-instruction object, so the force never
applied. p1:w2=c4 is accepted (forced=4) and recolours the same coalesced
web to a1, still 25 instructions, 20 masked. p1:w2=c7 is accepted
(forced=7) and recolours it to t0, still 25. p1:w2=s is accepted
(forced=-1) and spills, 28 instructions with a larger frame. Colour cannot
mint the three ring temporaries.

L145 delete-the-declared-ret is the or-chain: 25 instructions, one a0 web,
one copy at the epilogue, short-circuit branches to that join. Two-name
L51, goto, De Morgan, bang-bang, and a live a0 blocker all collapse to
that same object. The 29-instruction if-return-boolean family (declared
ret, assignment-in-condition, register, nested else, three distinct
locals) has the per-arm copies and inverted branches. Mixed
two-instruction normalisation is 27 instructions and 14 masked with the
join copy still on a0.

L144: address-of-arg0 is byte-identical to the or-chain. Address-of-the
test local is 27 instructions at delta 0 but a larger frame, extra stores,
one v1 web, 17 masked. Volatile ret is 35.

Screen: no candidate had 27 instructions and three ring-temp operands and
per-arm copies into the return register. Do not adopt the 25-instruction
coalesced or or the 29-instruction if-return family. Best remains the
retained early-return body: 108 bytes, 27 words, delta 0, 10 masked, first
+0x1C, frame 0x18, three relocations.

Next: a source form that keeps the or-chain branch sense while preventing
uopt from coalescing the three one-instruction normalisations into one
web, without a stack home or a second normalisation insn. Do not re-run
same-kind forces on web 2.

<!-- plateau-handoff:func_80028FCC:end -->
