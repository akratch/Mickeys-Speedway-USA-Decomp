#include "PR/ultratypes.h"
#include "n_audio/mbi.h"
#include "overlays/o100/motion.h"

typedef struct O100View {
    f32 scaleX;
    u8 pad04[0x10];
    f32 scaleY;
    u8 pad18[0x14];
    f32 depthScale;
} O100View;

extern O100View *overlay100GetViewReloc(void);
extern s16 *overlay100PrepareAnglesReloc(void);
extern f32 overlay100SinReloc(s32 angle);
extern f32 overlay100CosReloc(s32 angle);
extern void overlay100FinishCommandsReloc(Gfx **commands);
extern Gfx gOverlay100SegmentReloc[];

/*
 * Written from the listing (lane c-o066, 2026-10-07): GBI packet macros, the
 * two getters called with no arguments (their relocations name
 * func_8002468C(void) and camGetPtr(void); the values the target leaves in
 * a0-a3 at those calls are leftover webs, not arguments), plain colour
 * locals, `while (row--)` / `while (count--)`, the alpha-step clamp as an
 * if/else (the target's branch over an empty arm), and the count read before
 * the phase wrap. Four unused pointers declared first hold frame 0xC0.
 * 155 -> 113 masked at delta 0; the residual is the inner loop's FP colours
 * (inverseDepth copied through a temp) and the preheader's alpha/colour order.
 */
#ifdef NON_MATCHING
void overlay100DrawMotion(Gfx **dList, Overlay100Motion *motion) {
    void *unused0;
    void *unused1;
    void *unused2;
    void *unused3;
    f32 sinAngle, cosAngle, xScale, yScale;
    s32 green;
    s32 blue;
    f32 depthScale, depth, inverseDepth;
    O100View *view;
    Overlay100Vec3 *point;
    s16 *angle;
    s32 phase, row, count;
    s32 alphaStep;
    s32 alpha;
    s32 red;
    s32 x, y, progress;
    Gfx *commands;

    if (motion == NULL) {
        return;
    }
    commands = *dList;
    view = overlay100GetViewReloc();
    xScale = view->scaleX * 320.0f * 0.5f;
    yScale = view->scaleY * 240.0f * 0.5f;
    depthScale = view->depthScale;
    gSPDisplayList(commands++, gOverlay100SegmentReloc);
    progress = (motion->remaining << 16) / motion->duration;
    red = motion->colorA0 + (((motion->colorB0 - motion->colorA0) * progress) >> 16);
    green = motion->colorA1 + (((motion->colorB1 - motion->colorA1) * progress) >> 16);
    blue = motion->colorA2 + (((motion->colorB2 - motion->colorA2) * progress) >> 16);
    if (motion->remaining >= 64) {
        alphaStep = 255;
    } else {
        alphaStep = motion->remaining * 4;
    }
    angle = overlay100PrepareAnglesReloc();
    sinAngle = overlay100SinReloc(*angle + 0x8000);
    cosAngle = overlay100CosReloc(*angle + 0x8000);
    row = motion->bank;
    phase = motion->nextBank;
    while (row--) {
        gDPPipeSync(commands++);
        alpha = alphaStep * (3 - row);
        gDPSetPrimColor(commands++, 0, 0, red, green, blue, alpha / 3);
        point = motion->frames[phase];
        count = motion->count;
        phase++;
        if (phase >= 3) {
            phase = 0;
        }
        while (count--) {
            depth = point->z * sinAngle - point->x * cosAngle;
            if (depth < -10.0f) {
                inverseDepth = 1.0f / (depth * depthScale);
                x = (s32)((point->x * sinAngle + point->z * cosAngle) * xScale * inverseDepth) + 160;
                if ((u32)x < 320) {
                    y = 120 - (s32)(point->y * yScale * inverseDepth);
                    if ((u32)y < 240) {
                        gDPFillRectangle(commands++, x, y, x + 1, y + 1);
                    }
                }
            }
            point++;
        }
    }
    *dList = commands;
    overlay100FinishCommandsReloc(dList);
    gDPSetPrimColor((*dList)++, 0, 0, 255, 255, 255, 255);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o100/overlay100DrawMotion/func_overlay_100_F0000580_18DB2A8.s")
#endif

/* PLATEAU-HANDOFF:overlay100DrawMotion:start
 * symbol: overlay100DrawMotion
 * score: 113 differing words
 * frame: 0xC0
 * relocations: 7
 * first-mismatch: +0x194
 * summary: Void getters, GBI macros, if/else alpha clamp: 155 to 113. Left: preheader colour/alpha order, inner-loop FP colours.
 * PLATEAU-HANDOFF:overlay100DrawMotion:end
 */
