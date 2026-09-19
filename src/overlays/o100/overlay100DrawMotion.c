#include "PR/ultratypes.h"
#include "overlays/o100/motion.h"

typedef struct O100View {
    f32 scaleX;
    u8 pad04[0x10];
    f32 scaleY;
    u8 pad18[0x14];
    f32 depthScale;
} O100View;

typedef struct O100Command {
    u32 word0;
    u32 word1;
} O100Command;

extern O100View *overlay100GetViewReloc(O100Command **commandPtr);
extern s16 *overlay100PrepareAnglesReloc(O100Command *start, u8 colorA,
                                         u8 colorB, O100Command *end);
extern f32 overlay100SinReloc(s32 angle);
extern f32 overlay100CosReloc(s32 angle);
extern void overlay100FinishCommandsReloc(O100Command **commands);
extern u8 gOverlay100SegmentReloc[];

#define O100_SEGMENT(packet, address)                                    \
    {                                                                    \
        O100Command *macroCommand = (O100Command *)(packet);             \
        macroCommand->word0 = 0x06000000;                                \
        macroCommand->word1 = (u32)(address);                            \
    }
#define O100_SYNC(packet)                                                \
    {                                                                    \
        O100Command *macroCommand = (O100Command *)(packet);             \
        macroCommand->word0 = 0xE7000000;                                \
        macroCommand->word1 = 0;                                         \
    }
#define O100_PRIM(packet, color)                                         \
    {                                                                    \
        O100Command *macroCommand = (O100Command *)(packet);             \
        macroCommand->word1 = (color);                                   \
        macroCommand->word0 = 0xFA000000;                                \
    }
#define O100_FILL(packet, ulx, uly, lrx, lry)                            \
    {                                                                    \
        O100Command *macroCommand = (O100Command *)(packet);             \
        macroCommand->word0 = 0xF6000000 |                               \
            ((((u32)(lrx)) & 0x3FF) << 14) | ((((u32)(lry)) & 0x3FF) << 2); \
        macroCommand->word1 = ((((u32)(ulx)) & 0x3FF) << 14) |           \
            ((((u32)(uly)) & 0x3FF) << 2);                               \
    }

/*
 * Four unused pointers declared first close the 0xC0 frame (L99); four
 * projection scalars then the volatile color homes then commands last place
 * every stack slot on the target ladder. Packed RGB hoists stay two words
 * long. Resume with command/color lifetime that does not add instructions.
 */
#ifdef NON_MATCHING
void overlay100DrawMotion(O100Command **commandPtr, Overlay100Motion *motion) {
    void *unused0; /* L99: unused pointers declared first hold frame 0xC0 */
    void *unused1;
    void *unused2;
    void *unused3;
    f32 sinAngle, cosAngle, xScale, yScale;
    volatile s32 green;
    volatile s32 blue;
    f32 depthScale, depth, inverseDepth;
    O100View *view;
    Overlay100Vec3 *point;
    s16 *angleRecord;
    s32 phase, row, count;
    register s32 alphaStep;
    s32 alpha;
    register s32 red;
    s32 x, y, progress;
    O100Command *commands;

    if (motion == 0) return;
    commands = *commandPtr;
    view = overlay100GetViewReloc(commandPtr);
    xScale = view->scaleX * 320.0f * 0.5f;
    yScale = view->scaleY * 240.0f * 0.5f;
    depthScale = view->depthScale;

    O100_SEGMENT(commands++, gOverlay100SegmentReloc);
    progress = ((s32)motion->remaining << 16) / motion->duration;
    red = motion->colorA0 +
        (((motion->colorB0 - motion->colorA0) * progress) >> 16);
    green = motion->colorA1 +
        (((motion->colorB1 - motion->colorA1) * progress) >> 16);
    blue = motion->colorA2 +
        (((motion->colorB2 - motion->colorA2) * progress) >> 16);
    alphaStep = motion->remaining * 4;
    if (motion->remaining >= 64) alphaStep = 255;

    angleRecord = overlay100PrepareAnglesReloc(
        commands - 1, motion->colorA0, motion->colorA1, commands);
    sinAngle = overlay100SinReloc(*angleRecord + 0x8000);
    cosAngle = overlay100CosReloc(*angleRecord + 0x8000);
    row = motion->bank;
    phase = motion->nextBank;
    if (row != 0) {
        row--;
        alpha = alphaStep * (3 - row);
        do {
            O100_SYNC(commands++);
            O100_PRIM(commands++, ((u32)red << 24) |
                ((green & 0xFF) << 16) | ((blue & 0xFF) << 8) |
                ((alpha / 3) & 0xFF));
            point = motion->frames[phase];
            phase++;
            if (phase >= 3) phase = 0;
            count = motion->count;
            if (count-- != 0) {
                do {
                    depth = point->z * sinAngle - point->x * cosAngle;
                    if (depth < -10.0f) {
                        inverseDepth = 1.0f / (depth * depthScale);
                        x = (s32)(((point->x * sinAngle) +
                            (point->z * cosAngle)) * xScale * inverseDepth) + 160;
                        if ((u32)x < 320) {
                            y = 120 - (s32)(point->y * yScale * inverseDepth);
                            if ((u32)y < 240) {
                                O100_FILL(commands++, x, y, x + 1, y + 1);
                            }
                        }
                    }
                    point++;
                } while (count-- != 0);
            }
            alpha += alphaStep;
        } while (row-- != 0);
    }
    *commandPtr = commands;
    overlay100FinishCommandsReloc(commandPtr);
    O100_PRIM((*commandPtr)++, 0xFFFFFFFF);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o100/overlay100DrawMotion/func_overlay_100_F0000580_18DB2A8.s")
#endif

/* PLATEAU-HANDOFF:overlay100DrawMotion:start
 * symbol: overlay100DrawMotion
 * score: 155 differing words
 * frame: 0xC0
 * relocations: 7
 * first-mismatch: +0x0
 * summary: L99 unused pointers declared first close frame 0xC0 with exact homes. Packed RGB hoist is plus two words. Command/color lifetime remains.
 * PLATEAU-HANDOFF:overlay100DrawMotion:end
 */
