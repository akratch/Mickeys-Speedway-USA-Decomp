#include "PR/ultratypes.h"

typedef struct Overlay99Influence {
    f32 x0;
    f32 z0;
    f32 x1;
    f32 z1;
    f32 longitudinalScale;
    f32 widthScale;
    f32 edgeWidth;
    f32 intensity;
    f32 angleDegrees;
    f32 angleScale;
} Overlay99Influence;

typedef struct Overlay99GridPoint {
    s16 reserved00;
    s16 reserved02;
    s16 height;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} Overlay99GridPoint;

extern s32 gOverlay99CurrentGrid;
extern Overlay99GridPoint *gOverlay99Grids[];
extern s32 gOverlay99GridWidth;
extern s32 gOverlay99GridHeight;
extern s32 gOverlay99WidthMinusOne;
extern s32 gOverlay99HeightMinusOne;
extern s32 gOverlay99Arg4;
extern s32 gOverlay99Arg5;

#define G_ANGLE_UNITS_PER_DEGREE (65536.0f / 360.0f)

extern f32 overlay99AngleWave(s32 angle);
extern f32 overlay99AngleWavePhaseReloc(s32 angle);
extern f32 overlay99ProjectVector(f32 x, f32 y, f32 z, f32 dx, f32 dy);

/* Reusing the interpolation x carrier for the loop x (L115) restores the
 * 0xD0 frame. An empty `if (var_s0)` after the inner compact (overlay22
 * L100) plus volatile on the +0x88 plane coefficient close the two missing
 * words at that frame. */
#ifdef NON_MATCHING
void overlay99ApplySegment(Overlay99Influence *arg0, f32 arg1) {
    f32 spA0;
    f32 sp9C;
    f32 sp90;
    volatile f32 sp8C;
    volatile f32 sp88;
    f32 sp84;
    f32 sp7C;
    f32 temp_f0;
    f32 temp_f0_2;
    f32 temp_f0_3;
    f32 temp_f16;
    f32 temp_f20;
    f32 temp_f22;
    f32 temp_f24;
    f32 temp_f26;
    f32 temp_f28;
    f32 temp_f28_2;
    f32 temp_f2;
    f32 temp_f2_2;
    f32 temp_f30;
    f32 var_f20;
    s32 temp_s4;
    s32 var_s0;
    s32 var_s3;
    Overlay99GridPoint *var_s1;

    temp_f26 =
        ((arg0->x1 - (temp_f2 = arg0->x0)) * arg1) + temp_f2;
    temp_f28 =
        ((arg0->z1 - (temp_f16 = arg0->z0)) * arg1) + temp_f16;
    var_s1 = gOverlay99Grids[gOverlay99CurrentGrid];
    sp8C = temp_f16 - temp_f28;
    sp88 = temp_f26 - temp_f2;
    sp7C = -sp88;
    sp84 = -((temp_f2 * sp8C) + (temp_f16 * sp88));
    sp90 = -((temp_f26 * sp7C) + (temp_f28 * sp8C));
    spA0 = 1.0f / arg0->longitudinalScale;
    temp_s4 = (s32)(arg0->angleDegrees * G_ANGLE_UNITS_PER_DEGREE);
    sp9C = 1.0f / arg0->angleScale;
    if (var_s1 != 0) {
        var_s3 = 0;
        if (gOverlay99GridHeight > 0) {
            do {
                var_s0 = 0;
                if (gOverlay99GridWidth > 0) {
                    do {
                        temp_f26 =
                            (f32)(var_s0 - (gOverlay99WidthMinusOne >> 1)) *
                            (f32)gOverlay99Arg4;
                        temp_f28_2 =
                            (f32)((gOverlay99HeightMinusOne >> 1) - var_s3) *
                            (f32)gOverlay99Arg5;
                        temp_f0 = overlay99ProjectVector(
                            sp7C, sp8C, sp90, temp_f26, temp_f28_2);
                        if (temp_f0 > 0.0f) {
                            temp_f2_2 = temp_f0 * spA0;
                            if ((temp_f2_2 > 0.0f) && (temp_f2_2 < 1.0f)) {
                                temp_f30 = overlay99AngleWave(
                                    (s32)(temp_f0 * spA0 * 16384.0f));
                                temp_f22 = arg0->widthScale * temp_f2_2;
                                temp_f0_2 = overlay99ProjectVector(
                                    sp8C, sp88, sp84, temp_f26, temp_f28_2);
                                var_f20 = temp_f0_2;
                                if (temp_f0_2 < 0.0f) {
                                    var_f20 = -temp_f0_2;
                                }
                                if (var_f20 <= temp_f22) {
                                    temp_f20 = temp_f22 - var_f20;
                                    temp_f0_3 = arg0->edgeWidth;
                                    if (temp_f20 < temp_f0_3) {
                                        temp_f24 = overlay99AngleWave(
                                            (s32)((temp_f20 * 16384.0f) /
                                                  temp_f0_3));
                                        var_s1->height =
                                            (s16)(var_s1->height +
                                                  (s32)(
                                                      overlay99AngleWavePhaseReloc(
                                                          (s32)(temp_f20 *
                                                                65536.0f *
                                                                sp9C) +
                                                          temp_s4) *
                                                      (arg0->intensity *
                                                       temp_f24 * temp_f30)));
                                    }
                                }
                            }
                        }
                        var_s0++;
                        var_s1++;
                    } while (var_s0 < gOverlay99GridWidth);
                    /* Overlay22 empty-if L100: one extra occurrence of the
                     * inner index after the compact. */
                    if (var_s0) {
                    }
                }
                var_s3++;
            } while (var_s3 < gOverlay99GridHeight);
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o099/overlay99ApplySegment/func_overlay_099_F00002A0_18D9850.s")
#endif

/* PLATEAU-HANDOFF:overlay99ApplySegment:start
 * symbol: overlay99ApplySegment
 * score: 190/230 words
 * frame: 0xD0
 * relocations: 27
 * first-mismatch: +0x50
 * summary: Size 0 at frame 0xD0. L115 x-reuse, overlay22 empty if(var_s0), volatile sp88. 190 masked; stack homes still +0xB4 not +0xA0.
 * PLATEAU-HANDOFF:overlay99ApplySegment:end
 */
