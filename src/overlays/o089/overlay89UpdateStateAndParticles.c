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
extern s32 overlay34SpawnReloc(Overlay89Particle *particle);
extern void overlay89MaintainReloc(Overlay89Object *object,
                                   Overlay89EffectState *state);

/* DKR v77/v80 and JFG contain no exact donor for this state updater. */
/* Matched 2026-09-23 (lane B2-ov2). Four source facts, each read off the
 * instrumented allocator records against a forced-zero object first:
 *   - The loop is `if (count--) { randomScale = ...; do { } while (count--); }`
 *     with `count` assigned inside the particleCount test, so the load is one
 *     CSE'd temp copied into s0 and the guard keeps the target's dead copy.
 *   - overlay34Spawn returns a value: the post-decrement copy then takes v1
 *     rather than v0, as in the target.
 *   - secondaryHandle is declared right after particle (home sp+0x4C), and
 *     primaryHandle is read first (the target's prologue schedule).
 *   - The two empty do-while statements add four uopt blocks, raising the
 *     callee-save toll (L56, nBB/4) from 8 to 9. The hoisted &particle web
 *     (totalsave 10, cost toll + 1) then splits at 10 <= 10, and the address
 *     is rematerialised in the loop as the target does. Without them the web
 *     takes s1 and the function is three words too long. */
void overlay89UpdateStateAndParticles(Overlay89Object *object,
                                      volatile s32 updateRate) {
    f32 randomScale;
    Overlay89Particle particle;
    void *secondaryHandle;
    Overlay89EffectState *state;
    void *primaryHandle;
    s32 count;

    state = object->state;
    primaryHandle = state->primaryHandle;
    secondaryHandle = state->secondaryHandle;
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

                if (state->particleCount != 0) {
                    count = state->particleCount;
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
                    if (count--) {
                        randomScale = gOverlay89RandomScale[2];
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

    do { } while (0);
    do { } while (0);
    if (secondaryHandle != NULL) {
        overlay89MaintainReloc(object, state);
    }
    object->clearWord = 0;
}
