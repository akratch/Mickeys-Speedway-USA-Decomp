/*
 * Matrix and vector maths -- ROM 0x2B650-0x2BCD0 (VRAM 0x8002AA50).
 *
 * Six float-only functions with no rodata of their own: no jump tables and no
 * float literals, so the whole subsegment can become C without the .rodata
 * split that the rest of the static segment still needs. That is why this is
 * the first non-linker game TU here.
 *
 * Flags: -O2 -mips2 -32, the same src/main/ rule main/runlink.c established.
 *
 * NOTHING IN THIS FILE MATCHES, and the reason is not the source -- it is the
 * compiler. See the NONMATCHING-notes below.
 */

#include "PR/ultratypes.h"
#include "game/math.h"

typedef struct MatrixTransform {
    s16 rotation0;
    s16 rotation1;
    s16 rotation2;
    u8 pad06[2];
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
} MatrixTransform;

extern f32 func_8002A8BC(s32 angle);
extern f32 func_8002A8C0(s32 angle);

#ifdef NON_MATCHING
/* Workbench: structure-mismatch, 90 differing words, first mismatch +0x0.
 * Structural gap: 91 instructions/frame -0x48 versus target 74/-0x8; 27 aligned relocation sites differ.
 * Word-copying the fixed fields is the bounded best; six call results still spill around the ABI. */
/* PROVENANCE: adapted from Jet Force Gemini's public math_matrix implementation;
 * Mickey's own field offsets and call targets remain authoritative here. */
void func_8002AA50(MatrixTransform *trans, MtxF dest) {
    f32 cosX = func_8002A8C0(trans->rotation0);
    f32 sinX = func_8002A8BC(trans->rotation0);
    f32 cosY = func_8002A8C0(trans->rotation1);
    f32 sinY = func_8002A8BC(trans->rotation1);
    f32 cosZ = func_8002A8C0(trans->rotation2);
    f32 sinZ = func_8002A8BC(trans->rotation2);
    f32 scale = trans->scale;

    ((u32 *)dest)[3] = 0;
    ((u32 *)dest)[7] = 0;
    ((u32 *)dest)[11] = 0;
    ((u32 *)dest)[12] = ((u32 *)trans)[3];
    ((u32 *)dest)[13] = ((u32 *)trans)[4];
    ((u32 *)dest)[14] = ((u32 *)trans)[5];
    ((u32 *)dest)[15] = 0x3F800000;
    dest[0][0] = (((sinZ * sinX) + ((cosZ * cosX) * cosY)) * scale);
    dest[0][1] = (cosZ * sinY) * scale;
    dest[0][2] = (((sinX * cosZ) * cosY) - (sinZ * cosX)) * scale;
    dest[1][0] = (((sinZ * cosX) * cosY) - (sinX * cosZ)) * scale;
    dest[1][1] = (sinZ * sinY) * scale;
    dest[1][2] = (((sinZ * sinX) * cosY) + (cosZ * cosX)) * scale;
    dest[2][0] = (sinY * cosX) * scale;
    dest[2][1] = -cosY * scale;
    dest[2][2] = (sinY * sinX) * scale;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/matrix/func_8002AA50.s")
#endif
#ifdef NON_MATCHING
/* Workbench: structure-mismatch, 83 differing words, first mismatch +0x0.
 * Structural gap: 84 instructions/frame -0x48 versus target 67/-0x8; 27 relocation sites also differ.
 * Not shape-exact or permuter-ready; the six-call rotation body is retained for later structural work. */
/* PROVENANCE: adapted from Jet Force Gemini's public math_matrix implementation;
 * Mickey's own field offsets and call targets remain authoritative here. */
void func_8002AB78(MatrixTransform *trans, MtxF dest) {
    f32 cosX;
    f32 sinX;
    f32 cosY;
    f32 sinY;
    f32 cosZ;
    f32 sinZ;

    cosX = func_8002A8C0(trans->rotation0);
    sinX = func_8002A8BC(trans->rotation0);
    cosY = func_8002A8C0(trans->rotation1);
    sinY = func_8002A8BC(trans->rotation1);
    cosZ = func_8002A8C0(trans->rotation2);
    sinZ = func_8002A8BC(trans->rotation2);

    dest[0][3] = 0.0f;
    dest[1][3] = 0.0f;
    dest[2][3] = 0.0f;
    dest[3][0] = trans->x;
    dest[3][1] = trans->y;
    dest[3][2] = trans->z;
    dest[0][0] = ((cosZ * cosX) * cosY) + (sinZ * sinX);
    dest[0][1] = cosZ * sinY;
    dest[0][2] = ((cosZ * sinX) * cosY) - (cosX * sinZ);
    dest[1][0] = ((cosX * sinZ) * cosY) - (cosZ * sinX);
    dest[1][1] = sinZ * sinY;
    dest[1][2] = ((sinZ * sinX) * cosY) + (cosZ * cosX);
    dest[2][0] = cosX * sinY;
    dest[2][1] = -cosY;
    dest[2][2] = sinY * sinX;
    dest[3][3] = 1.0f;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/matrix/func_8002AB78.s")
#endif
#ifdef NON_MATCHING
/* Workbench: structure-mismatch, 118 differing words, first mismatch +0x0. */
/* Candidate shape: 119 instructions/frame -0x80 vs target 99/-0x8; six call relocations each. */
/* The exact JFG donor is hand-written assembly. Its odd-register allocation is outside stock IDO. */
/* PROVENANCE: adapted from Jet Force Gemini's public
 * asm/hasm/math_matrix.s matrix_XYZ_YPR_SCL; Mickey's field offsets and
 * helper call targets remain authoritative here. */
void func_8002AC84(MatrixTransform *trans, MtxF dest) {
    register f32 cosX;
    register f32 sinX;
    register f32 cosY;
    register f32 sinY;
    register f32 cosZ;
    register f32 sinZ;
    register f32 scale;

    cosX = func_8002A8C0(trans->rotation0);
    sinX = func_8002A8BC(trans->rotation0);
    cosY = func_8002A8C0(trans->rotation1);
    sinY = func_8002A8BC(trans->rotation1);
    cosZ = func_8002A8C0(trans->rotation2);
    sinZ = func_8002A8BC(trans->rotation2);

    scale = trans->scale;

    dest[0][3] = 0.0f;
    dest[1][3] = 0.0f;
    dest[2][3] = 0.0f;
    dest[3][3] = 1.0f;
    {
        register f32 col0_0;
        register f32 col0_1;
        register f32 col0_2;

        col0_0 = (sinX * sinZ - ((cosX * cosY) * cosZ)) * scale;
        col0_1 = ((-sinY) * cosZ) * scale;
        col0_2 = (cosX * sinZ + ((sinX * cosY) * cosZ)) * scale;
        dest[0][0] = col0_0;
        dest[1][0] = col0_1;
        dest[2][0] = col0_2;
        dest[3][0] = (trans->x * col0_0) + (trans->y * col0_1) + (trans->z * col0_2);
    }
    {
        register f32 col1_0;
        register f32 col1_1;
        register f32 col1_2;

        col1_0 = (sinX * cosZ + ((cosX * cosY) * sinZ)) * scale;
        col1_1 = (sinY * sinZ) * scale;
        col1_2 = (cosX * cosZ - ((sinX * cosY) * sinZ)) * scale;
        dest[0][1] = col1_0;
        dest[1][1] = col1_1;
        dest[2][1] = col1_2;
        dest[3][1] = (trans->x * col1_0) + (trans->y * col1_1) + (trans->z * col1_2);
    }
    {
        register f32 col2_0;
        register f32 col2_1;
        register f32 col2_2;

        col2_0 = (cosX * -sinY) * scale;
        col2_1 = cosY * scale;
        col2_2 = (sinX * sinY) * scale;
        dest[0][2] = col2_0;
        dest[1][2] = col2_1;
        dest[2][2] = col2_2;
        dest[3][2] = (trans->x * col2_0) + (trans->y * col2_1) + (trans->z * col2_2);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/matrix/func_8002AC84.s")
#endif
#ifdef NON_MATCHING
/* Workbench: structure-mismatch, 138 differing words, first mismatch +0x0.
 * Structural gap: 139 instructions/frame -0xa0 versus target 87/-0x8; 63 relocation sites also differ.
 * Not shape-exact or permuter-ready; the typed XYZ/YPR arithmetic remains a structural plateau. */
/* PROVENANCE: adapted from Jet Force Gemini's public math_matrix implementation;
 * Mickey's own field offsets and call targets remain authoritative here. */
void func_8002AE10(MatrixTransform *trans, MtxF dest) {
    f32 cosX;
    f32 sinX;
    f32 cosY;
    f32 sinY;
    f32 cosZ;
    f32 sinZ;
    f32 temp0;
    f32 temp1;
    f32 temp2;
    f32 temp3;
    f32 temp4;
    f32 temp5;
    f32 temp6;
    f32 temp7;
    f32 temp8;
    f32 temp9;
    f32 temp10;

    cosX = func_8002A8C0(trans->rotation0);
    sinX = func_8002A8BC(trans->rotation0);
    cosY = func_8002A8C0(trans->rotation1);
    sinY = func_8002A8BC(trans->rotation1);
    cosZ = func_8002A8C0(trans->rotation2);
    sinZ = func_8002A8BC(trans->rotation2);

    temp0 = sinX * sinZ;
    temp1 = sinX * cosZ;
    temp2 = sinY * sinZ;
    temp3 = cosX * sinZ;
    temp4 = cosX * cosZ;
    temp5 = sinX * sinY;
    temp6 = cosX * cosY;
    temp7 = sinX * cosY;
    temp8 = -sinY;
    temp9 = cosX * temp8;
    temp10 = temp8 * cosZ;

    temp0 -= temp6 * cosZ;
    temp1 += temp6 * sinZ;
    temp3 += temp7 * cosZ;
    temp4 -= temp7 * sinZ;

    dest[0][3] = 0.0f;
    dest[1][3] = 0.0f;
    dest[2][3] = 0.0f;
    dest[0][0] = temp0;
    dest[0][1] = temp1;
    dest[0][2] = temp9;
    dest[1][0] = temp10;
    dest[1][1] = temp2;
    dest[1][2] = cosY;
    dest[2][0] = temp3;
    dest[2][1] = temp4;
    dest[2][2] = temp5;
    dest[3][3] = 1.0f;
    dest[3][0] = (trans->x * temp0) + (trans->y * temp10) + (trans->z * temp3);
    dest[3][1] = (trans->x * temp1) + (trans->y * temp2) + (trans->z * temp4);
    dest[3][2] = (trans->x * temp9) + (trans->y * cosY) + (trans->z * temp5);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/matrix/func_8002AE10.s")
#endif
/*
 * NONMATCHING-notes for this whole file: the toolchain cannot emit the ROM's
 * floating-point register allocation.
 *
 * The ROM's float code uses ODD single-precision FP registers -- $f5, $f7,
 * $f9, $f11 and $f17 all appear in the 53 instructions at 0x8002AF6C alone.
 * At the project's flags the IDO 5.3 in tools/ido/ emits none: a four-term
 * dot-product test case compiled at every combination of -mips1/-mips2/-mips3
 * with -O1/-O2/-O3 produced zero odd single-precision FP registers in all nine
 * builds. Every candidate below therefore comes out using $f0/$f2/$f4/... and
 * differs from the ROM in almost every FP register name, with the instruction
 * schedule following from that. (It CAN emit them, under
 * -Wc,-mips3 -Wc,-fp32regs -- just not the ROM's ones. See below.)
 *
 * ROM-wide the evidence is 1727 odd FP register operands across 9 of the
 * static segment's asm files, out of 24984 FP operands in total. The GNU
 * assembler notices too: assembling asm/18FF0.s prints "Warning: float
 * register should be even" once per occurrence.
 *
 * WHAT IS SETTLED. No SGI IDO build can produce the ROM's allocation, and the
 * search for one is closed -- see docs/modules.md section 6.2 for the
 * mechanism. In short: this IDO's ugen does have SGI's -fp32regs, reachable as
 * `-Wc,-mips3 -Wc,-fp32regs` (the -Wo, form earlier sweeps used hands the flag
 * to uopt, which drops it), and with it MatrixMultiplyVec4 comes out at the
 * ROM's exact 53 instructions with odd registers -- but allocated across all
 * 32, including argument, return and unsaved callee-saved odd halves, where
 * the ROM confines itself to $f4-$f11 and $f16-$f18. That is structural: in
 * the matched decompilation of real IDO 7.1's ugen, -fp32regs is an
 * unconditional loop freeing all 16 odd registers, while the reservation logic
 * that protects live ones walks even register numbers only. Six IDO/MIPSpro
 * versions, 4.1 through 7.4.4, all choose the same all-32 set.
 *
 * So (a) is dead. Two explanations remain, and they are not exclusive:
 *
 *   (b) HAND-WRITTEN ASSEMBLY. Odd-register use is exactly what a human
 *       writing MIPS by hand produces, because the even-only constraint is a
 *       compiler convention rather than a hardware one here. This is NOT
 *       ruled out for these two functions and it is the cheapest thing to
 *       check first, because it would make them un-decompilable by design
 *       rather than blocked on a compiler. The per-file odd-operand density
 *       is NOT uniform, which is what a single-compiler story would predict:
 *
 *         61.3%  796/1299  asm/59DB0.s   <- rule this one out first
 *         50.0%   38/76    asm/4FC30.s
 *         43.5%   37/85    asm/59BF0.s
 *         40.1%  254/633   asm/nonmatchings/main/matrix/   <- THIS FILE
 *         24.2%  266/1100  asm/18FF0.s
 *         18.9%  252/1335  asm/2A250.s
 *         12.1%   44/365   asm/3B480.s
 *          2.9%    6/209   asm/33FA0.s
 *          2.7%   34/1257  asm/16140.s
 *
 *       A spread from 61% to 2.7% across nine files looks more like a mix of
 *       origins than like one allocator applied uniformly. Note where this
 *       file sits: 40%. That is high enough that hand-written assembly is a
 *       live explanation for the two functions below specifically, not just
 *       for the ROM in general -- and if it is the right one, they are
 *       un-decompilable by design and no compiler will fix them.
 *
 *   (c) A NON-IDO COMPILER for some or all of the game code. No positive
 *       evidence anywhere points at one; it is listed because nothing rules
 *       it out, not because anything suggests it.
 *
 * DO NOT re-sweep compiler flags or IDO versions for this file. The mechanism
 * above says in advance that every such sweep fails. What would reopen it is
 * named in docs/modules.md section 6.2, and neither item is something to wait
 * for.
 *
 * What IS believed correct is the C. Both bodies were derived from the ROM's
 * own multiply/add chains and reproduce the arithmetic exactly, and
 * MatrixMultiplyVec4 under -Wc,-mips3 -Wc,-fp32regs reproduces the ROM's
 * instruction count and kinds exactly, differing only in register names. Do
 * not rewrite them from scratch.
 *
 * func_8002B040 does not use odd FP registers. A fresh ownership-aware reproof
 * instead isolates one structural ABI-lowering difference: IDO spills the
 * second f32 formal and reloads it, while the 34-word target moves all three
 * incoming GPR bit patterns directly into FP registers. No stock flag or
 * source-faithful type/expression spelling tested below removes that extra
 * instruction; the public DKR/JFG matrix-transform assembly is donor context,
 * not evidence that this target was compiled from C.
 */
#ifdef NON_MATCHING
/*
 * dst = m * src, treating src as a column vector:
 *   dst[i] = src[0]*m[i][0] + src[1]*m[i][1] + src[2]*m[i][2] + src[3]*m[i][3]
 *
 * The four src components are loaded once at the top and reused across all
 * four output rows; that is IDO's own common-subexpression elimination, not
 * something the source has to spell out.
 */
void MatrixMultiplyVec4(MtxF m, f32 *src, f32 *dst) {
    f32 x;
    f32 y;
    f32 z;
    f32 w;

    x = src[0];
    y = src[1];
    z = src[2];
    w = src[3];
    dst[0] = x * m[0][0] + y * m[0][1] + z * m[0][2] + w * m[0][3];
    dst[1] = x * m[1][0] + y * m[1][1] + z * m[1][2] + w * m[1][3];
    dst[2] = x * m[2][0] + y * m[2][1] + z * m[2][2] + w * m[2][3];
    dst[3] = x * m[3][0] + y * m[3][1] + z * m[3][2] + w * m[3][3];
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/main/matrix/func_8002AF6C.s")
#endif
/*
 * Rotate a direction by the matrix's upper 3x3, the other way round from
 * MatrixMultiplyVec4: the input scales whole *rows* rather than being dotted
 * with them, and the translation row is ignored.
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
#pragma GLOBAL_ASM("asm/nonmatchings/main/matrix/func_8002B040.s")
#endif

/* PLATEAU-HANDOFF:func_8002B040:start
 * symbol: func_8002B040
 * score: 34 differing words
 * frame: frameless
 * relocations: 0
 * first-mismatch: +0x0
 * summary: Extra arg2 home at +0x0; c28 stays infinite for the third incoming scalar at -O2. Four new spellings flat.
 * PLATEAU-HANDOFF:func_8002B040:end
 */

/* PLATEAU-HANDOFF:func_8002AA50:start
 * symbol: func_8002AA50
 * score: 90 differing words
 * frame: 0x48
 * relocations: 6
 * first-mismatch: +0x0
 * summary: Hand-assembly pattern retains odd caller-saved FP results outside stock IDO; retain fallback.
 * PLATEAU-HANDOFF:func_8002AA50:end
 */

/* PLATEAU-HANDOFF:func_8002AB78:start
 * symbol: func_8002AB78
 * score: 83 differing words
 * frame: 0x48
 * relocations: 6
 * first-mismatch: +0x0
 * summary: Hand-assembly donor requires odd caller-saved FP results that stock IDO cannot emit; retain fallback.
 * PLATEAU-HANDOFF:func_8002AB78:end
 */

/* PLATEAU-HANDOFF:func_8002AC84:start
 * symbol: func_8002AC84
 * score: 118 differing words
 * frame: 0x80
 * relocations: 6
 * first-mismatch: +0x0
 * summary: Hand-written assembly donor uses odd FP results beyond stock IDO; only one call identity aligns.
 * PLATEAU-HANDOFF:func_8002AC84:end
 */

/* PLATEAU-HANDOFF:func_8002AE10:start
 * symbol: func_8002AE10
 * score: 138 differing words
 * frame: 0xA0
 * relocations: 6
 * first-mismatch: +0x0
 * summary: JFG-identical hand assembly uses odd FP registers and frame 0x8. Reopen only for reservation-aware patched codegen or a proven matching C donor.
 * PLATEAU-HANDOFF:func_8002AE10:end
 */
