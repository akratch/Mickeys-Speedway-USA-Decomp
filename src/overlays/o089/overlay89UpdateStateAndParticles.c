#include "PR/ultratypes.h"

typedef struct Overlay89EffectState {
    u8 pad00[3];
    u8 particleCount;
    u8 red;
    u8 green;
    u8 blue;
    u8 intensity;
    u8 frozen;
    u8 pad09;
    s16 timer;
    u8 pad0C[0x14];
    void *primaryHandle;
    u8 pad24[8];
    void *secondaryHandle;
} Overlay89EffectState;

typedef struct Overlay89Object {
    s16 angleA;
    s16 angleB;
    u8 pad04[8];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x22];
    u8 active;
    u8 pad3B[0x29];
    Overlay89EffectState *state;
    u8 pad68[0x1C];
    s32 clearWord;
} Overlay89Object;

typedef struct Overlay89Particle {
    s16 angleA;
    s16 angleB;
    f32 speed;
    f32 x;
    f32 y;
    f32 z;
    f32 size;
    s16 lifetime;
    s8 type;
    u8 pad1B;
    s32 colorA;
    s32 colorB;
    s32 rotation;
    u32 flagsA;
    u32 flagsB;
    u32 flagsC;
} Overlay89Particle;

extern f32 gOverlay89RandomScale[];
extern void overlay89UpdateReloc(void *object, Overlay89EffectState *state,
                                 s32 updateRate);
extern void overlay89SetPrimaryReloc(void *handle, u8 value);
extern void overlay89SetColorReloc(void *handle, u8 red, u8 green, u8 blue,
                                   s32 intensity);
extern s32 overlay89RandomReloc(s32 minimum, s32 maximum);
extern void overlay34SpawnReloc(Overlay89Particle *particle);
extern void overlay89MaintainReloc(Overlay89Object *object,
                                   Overlay89EffectState *state);

/* DKR v77/v80 and JFG contain no exact donor for this state updater. */
/* 2026-09-18: s32 count, randomScale first, early z. Frame 0x88, 85 masked,
 * size still +4. Extra s3 is type-1 &particle (web 122); force-split
 * rematerializes. See the plateau handoff. */
#ifdef NON_MATCHING
void overlay89UpdateStateAndParticles(Overlay89Object *object,
                                      volatile s32 updateRate) {
    f32 randomScale;
    Overlay89Particle particle;
    Overlay89EffectState *state;
    void *primaryHandle;
    void *secondaryHandle;
    s32 count;

    state = object->state;
    secondaryHandle = state->secondaryHandle;
    primaryHandle = state->primaryHandle;
    overlay89UpdateReloc(object, state, updateRate);

    if (state->frozen == 0) {
        if (state->timer != 0) {
            state->timer -= updateRate;
            if (state->timer <= 0) {
                state->timer = 0;
                object->active = 0;
                if (primaryHandle != NULL) {
                    overlay89SetPrimaryReloc(primaryHandle, state->intensity);
                }
                if (secondaryHandle != NULL) {
                    overlay89SetColorReloc(secondaryHandle, state->red,
                                           state->green, state->blue,
                                           state->intensity);
                }

                count = state->particleCount;
                if (count != 0) {
                    particle.x = object->x;
                    particle.y = object->y;
                    particle.z = object->z;
                    particle.lifetime = 0x80;
                    particle.type = 0xA;
                    particle.colorA = -0x7F01;
                    particle.flagsA = 0xFF0000FF;
                    particle.colorB = -0x7F01;
                    particle.flagsB = 0xFF0000FF;
                    particle.rotation = -0x8000;
                    particle.flagsC = 0xFF000000;
                    if (count != 0) {
                        randomScale = gOverlay89RandomScale[2];
                        count--;
                        do {
                            particle.angleA =
                                overlay89RandomReloc(-0x2000, 0x2000) +
                                object->angleA;
                            particle.angleB =
                                overlay89RandomReloc(-0x2000, 0x2000) +
                                object->angleB;
                            particle.speed =
                                overlay89RandomReloc(10, 20) * randomScale;
                            particle.size =
                                overlay89RandomReloc(12, 25) * randomScale;
                            overlay34SpawnReloc(&particle);
                        } while (count--);
                    }
                }
            }
        } else {
            object->active = 1;
            state->timer = overlay89RandomReloc(30, 480);
            if (primaryHandle != NULL) {
                overlay89SetPrimaryReloc(primaryHandle, 0);
            }
            if (secondaryHandle != NULL) {
                overlay89SetColorReloc(secondaryHandle, state->red,
                                       state->green, state->blue, 0);
            }
        }
    }

    if (secondaryHandle != NULL) {
        overlay89MaintainReloc(object, state);
    }
    object->clearWord = 0;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o089/overlay89UpdateStateAndParticles/func_overlay_089_F00005A4_18D47D4.s")
#endif

/* PLATEAU-HANDOFF:overlay89UpdateStateAndParticles:start
 * symbol: overlay89UpdateStateAndParticles
 * score: 85 differing words
 * frame: 0x88
 * relocations: 14
 * first-mismatch: +0x0
 * summary: Frame 0x88 at 85 masked; extra s3 is type-1 particle address web 122; split rematerializes with secondaryHandle 8 bytes low and size -8.
 * PLATEAU-HANDOFF:overlay89UpdateStateAndParticles:end
 */
