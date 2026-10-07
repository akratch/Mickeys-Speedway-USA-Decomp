/*
 * Matrix maths -- ROM 0x2BC40-0x2BCD0 (VRAM 0x8002B040), one function and
 * eight bytes of alignment.
 *
 * Working split from the former main/matrix TU. Jet Force Gemini's
 * hand-written `hasm/math_matrix` object, whose routines two to five are
 * byte-identical to the verified main/math_matrix run just before this, has
 * no counterpart for this function and is followed directly by memory.c.
 * That position is structural evidence that this routine was appended to the
 * same hand-written file in Mickey's revision; it is not proof, so the C
 * candidate below stays queued.
 *
 * Flags: -O2 -mips2 -32.
 */

#include "PR/ultratypes.h"
#include "game/math.h"

/*
 * Rotate a direction by the matrix's upper 3x3, the other way round from
 * func_8002AF6C (JFG matrixTransposeVectorMultiply): the input scales whole
 * *rows* rather than being dotted with them, and the translation row is
 * ignored.
 *
 *   *dstX = x*m[0][0] + y*m[1][0] + z*m[2][0]   (and likewise for Y, Z)
 *
 * The three scalars arrive in a1/a2/a3 as integers and are moved across with
 * mtc1, which is just o32: because the first argument is a pointer, no
 * floating-point argument register is used at all. The three destinations are
 * the stack arguments at 0x10/0x14/0x18(sp).
 */
#ifdef NON_MATCHING
/*
 * Configured -O2 emits 35 words: two `mtc1` of a1/a3 and `sw`+`lwc1` of a2
 * through 8(sp). Align names that extra word at +0x0 (L155); the frame
 * census's extra slot is +0x8 with one store and one load. Driver -O3
 * (not phase-all-O3, which appends -O3 after -O2 and is inert) emits 34
 * words and three `mtc1`. The same 34-word object is reachable at -O2 by
 * CDX_FORCE=p2:w15=c28 (accepted forced=28): web 15 is the arg2 float,
 * class-2, totalsave 3, no-color because bestcost is the callee 4.0 and
 * caller f16 (c28) is infinite-cost for the three incoming-scalar webs.
 * f12/f14 colour the other two at cost 0. Extra copy, L144 address form,
 * L97/goto regions, store-kill, register formals, K&R, 2-D indexing,
 * mul-by-1 copies, L160 (no flatMatrix), L99 unused pointer/f32 first,
 * leftover OR-zero, overlay22 empty-if, overlay40 comma-assign, and
 * overlay41-style remat-delete (arg2 products first) all keep the 2-of-3
 * split; they rotate which formal spills or grow the function. Force
 * split of web 15 is accepted and still no-color. Matrix-first copies
 * plus the c28 force score 18 masked at +0x44, matching the best driver
 * -O3 body. Do not move the TU to -O3.
 */
void func_8002B040(MtxF matrix, f32 arg1, f32 arg2, f32 arg3,
                   f32 *arg4, f32 *arg5, f32 *arg6) {
    f32 *flatMatrix;

    flatMatrix = (f32 *)matrix;
    *arg4 = arg1 * flatMatrix[0] + arg2 * flatMatrix[4] + arg3 * flatMatrix[8];
    *arg5 = arg1 * flatMatrix[1] + arg2 * flatMatrix[5] + arg3 * flatMatrix[9];
    *arg6 = arg1 * flatMatrix[2] + arg2 * flatMatrix[6] + arg3 * flatMatrix[10];
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/matrix_2BC40/func_8002B040.s")
#endif

/* PLATEAU-HANDOFF:func_8002B040:start
 * symbol: func_8002B040
 * score: 34 differing words
 * frame: frameless
 * relocations: 0
 * first-mismatch: +0x0
 * summary: Own TU now. -O2 keeps the arg2 stack home (c28 infinite on incoming scalars); -O3 floors at 18, delta 0, outer add operand order at +0x2C.
 * PLATEAU-HANDOFF:func_8002B040:end
 */
