#include "overlays/o100/motion.h"

extern s32 D_80079278;
extern Overlay100Motion *gOverlay100EntriesReloc[];
extern f32 gOverlay100VelocityScaleReloc[];
extern void *mmAlloc(s32 size, s32 tag);
extern s32 func_800299E8(s32 minimum, s32 maximum);
extern void mathOneFloatPY(s16 *angles, Overlay100Vec3 *velocity);

Overlay100Motion *overlay100InitializeMotion(
    f32 x, f32 y, f32 z, s32 count, s32 colorB0, s32 colorB1, s32 colorB2,
    s32 colorA0, s32 colorA1, s32 colorA2, f32 durationSeconds) {
    Overlay100Motion *motion;
    Overlay100Vec3 *velocity;
    s32 remaining;
    s32 frameIndex;
    s16 angles[2];
    s32 velocityBytes;
    f32 velocityScale;

    if (D_80079278 >= 16) {
        return 0;
    }

    velocityBytes = count * (s32)sizeof(Overlay100Vec3);
    motion = mmAlloc(sizeof(Overlay100Motion) + velocityBytes * 4,
                                  0x87);
    if (motion != 0) {
        motion->bank = 1;
        motion->nextBank = 0;
        motion->phase = 0;
        motion->count = count;
        motion->duration = (s16)(s32)(durationSeconds * 60.0f);
        motion->remaining = motion->duration;
        motion->colorB0 = colorB0;
        motion->colorB1 = colorB1;
        motion->colorB2 = colorB2;
        motion->colorA0 = colorA0;
        motion->colorA1 = colorA1;
        motion->colorA2 = colorA2;
        motion->velocity = velocity = (Overlay100Vec3 *)(motion + 1);

        remaining = motion->count;
        if (remaining--) {
            velocityScale = gOverlay100VelocityScaleReloc[0];
            do {
                angles[0] = func_800299E8(0, 0xFFFF);
                angles[1] = func_800299E8(0, 0xFFFF);
                velocity->z =
                    -(f32)func_800299E8(0x180, 0x7F) * velocityScale;
                mathOneFloatPY(angles, velocity);
                velocity++;
            } while (remaining--);
        }

        velocity = (Overlay100Vec3 *)((u8 *)motion->velocity + velocityBytes);
        frameIndex = 0;
        do {
            remaining = motion->count;
            motion->frames[frameIndex] = velocity;
            frameIndex++;
            if (remaining--) {
                do {
                    velocity->x = x;
                    velocity->y = y;
                    velocity->z = z;
                    velocity++;
                } while (remaining--);
            }
        } while (frameIndex != 3);

        gOverlay100EntriesReloc[D_80079278] = motion;
        D_80079278++;
    }
    return motion;
}
