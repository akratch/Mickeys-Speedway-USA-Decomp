/*
 * Matrix maths -- ROM 0x2B650-0x2B778 (VRAM 0x8002AA50), one function.
 *
 * Working split. In Jet Force Gemini's tree this whole region is one
 * hand-written object, `hasm/math_matrix` (five routines, followed directly
 * by memory.c). The four routines after this one are byte-identical to that
 * object's routines two to five and are kept as verified assembly in
 * main/math_matrix (mickey.us.yaml, verified_asm.us.txt). func_8002B040,
 * which JFG does not have, is in main/matrix_2BC40.c.
 *
 * This function is JFG's first routine, matrix_SCL_RPY_XYZ, in a shorter
 * revision: 0x128 bytes against JFG's 0x148. Measured against JFG's built
 * object with the R_MIPS_26 fields masked, the first 18 words (+0x0..+0x48:
 * the frame and the three Cosf/Sinf call pairs) are identical and the body
 * after +0x48 is a different instruction sequence (aligned similarity 0.42),
 * so byte identity is not available and it is not in the verified ledger.
 * Like its four neighbours it uses odd single-precision FP registers, which
 * no IDO build emits (docs/modules.md section 6.2); it is very probably
 * hand-written too, but that is not proved.
 *
 * Flags: -O2 -mips2 -32. The Makefile trims .text to 0x128.
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

/* PLATEAU-HANDOFF:func_8002AA50:start
 * symbol: func_8002AA50
 * score: 90 differing words
 * frame: 0x48
 * relocations: 6
 * first-mismatch: +0x0
 * summary: Hand-assembly pattern retains odd caller-saved FP results outside stock IDO; retain fallback.
 * PLATEAU-HANDOFF:func_8002AA50:end
 */
