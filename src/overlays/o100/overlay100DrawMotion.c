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
 * func_8002468C(void) and camGetPtr(void); what the target leaves in a0-a3
 * at those calls are leftover webs, not arguments), plain colour locals,
 * `while (row--)` / `while (count--)`, the alpha-step clamp as an if/else
 * (the target's branch over an empty arm), the count read before the frame
 * pointer, and the alpha product inline in the colour packet (so the packed
 * colour word is numbered, and emitted in the setup, first). In the inner
 * loop the depth and the reciprocal are expressions written at each use, so
 * uopt shares them as single webs (the reciprocal computed straight into
 * f16), and point->y is read into a local at the top of the visible branch,
 * which is the shipped early f18 load. The unreferenced locals place the
 * frame (0xC0) and the green/blue/command spill homes.
 */
void overlay100DrawMotion(Gfx **dList, Overlay100Motion *motion) {
    void *unused0; /* unreferenced: frame 0xC0 and the target's homes */
    void *unused1;
    void *unused2;
    void *unused3;
    f32 sinAngle, cosAngle, xScale, yScale;
    s32 green;
    s32 blue;
    f32 py;
    void *unused4;
    f32 depthScale, unused5;
    O100View *view;
    Overlay100Vec3 *point;
    s16 *angle;
    s32 phase, row, count;
    s32 alphaStep;
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
        gDPSetPrimColor(commands++, 0, 0, red, green, blue, (alphaStep * (3 - row)) / 3);
        count = motion->count;
        point = motion->frames[phase];
        phase++;
        if (phase >= 3) {
            phase = 0;
        }
        while (count--) {
            if ((-(point->x * cosAngle) + point->z * sinAngle) < -10.0f) {
                py = point->y;
                x = (s32)((point->x * sinAngle + point->z * cosAngle) * xScale * (1.0f / ((-(point->x * cosAngle) + point->z * sinAngle) * depthScale))) + 160;
                if ((u32)x < 320) {
                    y = 120 - (s32)(py * yScale * (1.0f / ((-(point->x * cosAngle) + point->z * sinAngle) * depthScale)));
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
