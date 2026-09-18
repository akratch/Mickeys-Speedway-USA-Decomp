#include "overlays/overlay_012.h"
#include "game/track.h"

typedef struct Overlay12TrackHeight {
    u8 pad00[0x24];
    s16 height;
} Overlay12TrackHeight;

extern f32 gOverlay12Gravity;
extern f32 gOverlay12Acceleration;
extern f32 gOverlay12RandomScale;
extern s32 func_8001291C(f32 *previous, f32 *current,
                         f32 *result, s32 mask, s32 flags);
extern s32 mathRnd(s32 minimum, s32 maximum);
extern f32 sqrtf(f32 value);
extern s16 Arctanf(f32 x, f32 y);
extern f32 func_8002A8BC(s32 angle);
extern f32 func_8002A8C0(s32 angle);
extern s32 func_80036544(void *entry, s32 *mode, s32 animationId,
                         void *state, s32 updateRate);

/*
 * JFG's bloodSpurtUpdateAll is the closest masked-skeleton sibling, but its
 * public source is GLOBAL_ASM. This body is reconstructed from Mickey only.
 *
 * Size 0, frame 0x110, 226 masked. Named gRate yields delay-slot cvt without
 * an f22 copy; indexed effects close the walking-pointer extra; unsigned
 * sltiu spells the particle test; a volatile value pointer plus collision[10]
 * yields bnel plus sb. Colour landscape floor 224.
 */
#ifdef NON_MATCHING
void func_overlay_012_F00003A8_186D628(s32 updateRate) {
    /*
     * PROVENANCE: JFG's public src/camlight.c uses an 11-float output buffer
     * for trackNearestIntersection. Mickey's own accesses use indices 0-9;
     * L112 solves the unobservable extra slot for the volatile pointer home.
     */
    Overlay12TrackHeight *track;
    Overlay12Effect *effect;
    Overlay12Particle *particle;
    f32 updateRateF = (f32)updateRate;
    f32 minimumHeight;
    f32 tempA;
    f32 tempB;
    f32 tempC;
    f32 xBasis;
    f32 sineRandom;
    f32 cosineRandom;
    f32 sinePitch;
    f32 cosinePitch;
    f32 sineYaw;
    f32 cosineYaw;
    s16 pitch;
    s16 yaw;
    s16 randomAngle;
    s32 mode = 1;
    s32 i;
    f32 collision[10];

    track = (Overlay12TrackHeight *)trackGetTrack();
    minimumHeight = (f32)track->height - 1000.0f;
    for (i = 0; i < 64; i++) {
        effect = &gOverlay12Effects[i];
        switch (effect->active) {
        case 1:
            tempA = effect->y2;
            {
                f32 gRate = gOverlay12Gravity * updateRateF;
                effect->y0 += (tempA * updateRateF) + (gRate * updateRateF);
            }
            if (effect->y0 < minimumHeight) {
                effect->active = 0;
                gOverlay12EffectCount--;
            } else {
                effect->x0 += effect->x2 * updateRateF;
                effect->z0 += effect->z2 * updateRateF;
                effect->y2 = tempA + (gOverlay12Acceleration * updateRateF);
                if ((((i & 1) != 0) && (gOverlay12Value1598 != 0)) ||
                    (((i & 1) == 0) && (gOverlay12Value1598 == 0))) {
                    if (func_8001291C(&effect->x1, &effect->x0, collision,
                                      0x10000, 0) != 0) {
                        effect->collisionX = collision[4];
                        effect->collisionY = collision[5];
                        effect->collisionZ = collision[6];
                        effect->collisionValue = collision[7];
                        effect->x0 = collision[1];
                        effect->y0 = collision[2];
                        effect->z0 = collision[3];
                        effect->scaleY =
                            (255 - (((((u32 *)collision)[9] >> 24) & 7) << 5)) << 5;
                        {
                            volatile f32 *vp = &effect->value;
                            *vp *=
                                2.0f + ((f32)(mathRnd(0, 255) - 128) *
                                        gOverlay12RandomScale);
                        }

                        if (((s32 *)collision)[0] == 0) {
                            pitch = Arctanf(collision[5],
                                            sqrtf((collision[6] * collision[6]) +
                                                  (collision[4] * collision[4]))) -
                                    0x4000;
                            yaw = Arctanf(-collision[4], -collision[6]);
                            randomAngle = (s16)mathRnd(-0x8000, 0x7FFF);
                            sineRandom = func_8002A8BC(randomAngle);
                            cosineRandom = func_8002A8C0(randomAngle);
                            sinePitch = func_8002A8BC(-pitch);
                            cosinePitch = func_8002A8C0(-pitch);
                            sineYaw = func_8002A8BC(yaw);
                            cosineYaw = func_8002A8C0(yaw);
                            tempA = ((-10.0f * sineRandom) +
                                     (10.0f * cosineRandom)) * effect->value;
                            tempB = ((10.0f * sineRandom) -
                                     (-10.0f * cosineRandom)) * effect->value;
                            tempC = tempB * sinePitch;
                            effect->collided = 1;
                            xBasis = tempB * cosinePitch;
                            effect->vertexY0 = (s16)xBasis;
                            effect->vertexX0 =
                                (s16)((tempA * sineYaw) +
                                      (tempC * cosineYaw));
                            effect->vertexZ0 =
                                (s16)((tempC * sineYaw) -
                                      (tempA * cosineYaw));

                            tempA = ((10.0f * sineRandom) +
                                     (10.0f * cosineRandom)) * effect->value;
                            tempB = ((10.0f * sineRandom) -
                                     (10.0f * cosineRandom)) * effect->value;
                            tempC = tempB * sinePitch;
                            xBasis = tempB * cosinePitch;
                            effect->vertexY1 = (s16)xBasis;
                            effect->vertexX1 =
                                (s16)((tempA * sineYaw) +
                                      (tempC * cosineYaw));
                            effect->vertexZ1 =
                                (s16)((tempC * sineYaw) -
                                      (tempA * cosineYaw));
                        } else {
                            effect->collided = 0;
                        }
                        effect->active = 2;
                    } else {
                        effect->x1 = effect->x0;
                        effect->y1 = effect->y0;
                        effect->z1 = effect->z0;
                    }
                }
            }
            break;
        case 2:
            effect->scaleX += (effect->scaleY - effect->scaleX) >> 1;
            if (func_80036544(gOverlay12Resource5, &mode, 12,
                              &effect->zero, updateRate) != 0) {
                effect->active = 3;
            }
            break;
        case 3:
            effect->lifetime -= updateRate;
            if (effect->lifetime < 0) {
                effect->active = 0;
                gOverlay12EffectCount--;
            }
            break;
        }
    }

    gOverlay12Value1598 ^= 1;
    mode = 1;
    particle = gOverlay12Particles;
    for (i = 0; i < 5; i++, particle++) {
        if (((u32)((u32)particle->active < 1u)) == 0) {
            if (func_80036544(gOverlay12Resource5, &mode, 15,
                              &particle->velocity, updateRate) != 0) {
                particle->active = 0;
                gOverlay12ParticleCount--;
            }
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o012/func_overlay_012_F00003A8_186D628/func_overlay_012_F00003A8_186D628.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_012_F00003A8_186D628:start
 * symbol: func_overlay_012_F00003A8_186D628
 * score: 226 differing words
 * frame: 0x110
 * relocations: 33
 * first-mismatch: +0x50
 * summary: Size 0, frame 0x110, masked 226. Colour floor 224. Blocker: volatile value pointer vs collision-load hoist; physics loads; vertex nop plus sh.
 * PLATEAU-HANDOFF:func_overlay_012_F00003A8_186D628:end
 */
