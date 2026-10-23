#include "PR/ultratypes.h"
#include "game/math.h"

extern f32 D_80083F60;

/* PROVENANCE: adapted from Jet Force Gemini's public libultra decomp,
 * libultra/src/gu/perspective.c:guPerspectiveF; Mickey's angle helpers,
 * constants, and linked bytes remain authoritative. */
/* The declared-local list is load-bearing here, not decoration. Its length
 * sizes the 0x38 frame and its order fixes each home at frame_top - 4k, so
 * `sine` lands at sp+0x20 and `angle` at sp+0x1C only when they are the sixth
 * and seventh declarations. `i` is the survivor of the SGI original's scale
 * loop, which JFG's variant also still declares after folding the loop away;
 * the compiler never materialises it, but dropping it shortens the frame to
 * 0x30. */
void func_8004FAD0(MtxF mf, u16 *perspNorm, f32 fovy, f32 aspect,
                   f32 nearPlane, f32 farPlane, f32 scale) {
    s32 i;
    f32 cot;
    f32 sumZ;
    f32 deltaZ;
    f32 aspectCot;
    f32 sine;
    s32 angle;

    angle = (s32)(fovy * D_80083F60);
    sine = func_8002A8BC(angle);
    cot = (sine / func_8002A8C0(angle)) * scale;
    aspectCot = cot / aspect;
    deltaZ = scale / (nearPlane - farPlane);
    mf[0][0] = aspectCot;
    mf[0][1] = 0.0f;
    mf[0][2] = 0.0f;
    mf[0][3] = 0.0f;
    mf[1][0] = 0.0f;
    mf[1][1] = cot;
    mf[1][2] = 0.0f;
    mf[1][3] = 0.0f;
    mf[2][0] = 0.0f;
    mf[2][1] = 0.0f;
    sumZ = nearPlane + farPlane;
    mf[2][2] = sumZ * deltaZ;
    mf[2][3] = -1.0f;
    mf[3][0] = 0.0f;
    mf[3][1] = 0.0f;
    mf[3][2] = (2.0f * nearPlane * farPlane) * deltaZ;
    mf[3][3] = 0.0f;

    if (perspNorm != (u16 *)NULL) {
        if (sumZ <= 2.0f) {
            *perspNorm = (u16)0xFFFF;
        } else {
            *perspNorm = (s32)(131072.0f / sumZ);
            if (*perspNorm <= 0) {
                *perspNorm = (u16)0x0001;
            }
        }
    }
}
