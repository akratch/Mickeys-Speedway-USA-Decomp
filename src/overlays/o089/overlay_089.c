#include "overlays/overlay_089.h"

/*
 * DKR v77/v80 and JFG have no exact donor for this effect update. The
 * per-file R4300 multiply scheduling flag is required for the shipped hazard
 * nop.
 */
static void overlay89UpdateEffect(const Overlay89Object *transform,
                                  Overlay89EffectState *effect) {
    f32 x;
    f32 y;
    f32 z;
    f32 trigA0;
    f32 trigB0;
    f32 trigA1;
    f32 trigB1;
    f32 scaleValue;
    volatile f32 scaleArg; /* load-bearing: without it the whole module differs (measured) */

    if (effect->primaryHandle == 0) {
        return;
    }

    trigA0 = func_8002A8C0(transform->angleA);
    trigB0 = func_8002A8BC(transform->angleA);
    trigA1 = func_8002A8C0(transform->angleB);
    trigB1 = func_8002A8BC(transform->angleB);

    scaleValue = (scaleArg = effect->scale);
    scaleValue *= gOverlay89EffectScale;
    z = effect->radius;
    trigB1 = z * trigB1;
    x = transform->x - (trigB1 * trigA0);
    y = transform->y + (effect->radius * trigA1);
    z = transform->z - (trigB1 * trigB0);

    if (effect->secondaryHandle != NULL) {
        trackLightMove(effect->secondaryHandle, x, y, z);
    } else {
        effect->secondaryHandle = trackLightAdd(
            x,
            y,
            z,
            scaleArg,
            scaleValue,
            (effect->red * effect->intensity) >> 8,
            (effect->green * effect->intensity) >> 8,
            (effect->blue * effect->intensity) >> 8);
    }
}

/* DKR v77/v80 and JFG contain no exact donor for this rule evaluator. */
static s32 overlay89Evaluate(Overlay89EffectState *rule) {
    switch (rule->mode) {
        case 0:
            return rule->enabled;
        case 1:
            return func_8002905C(rule->index, rule);
        case 2:
            return func_8002905C(rule->index, rule) == 0;
        default:
            return 1;
    }
}

/*
 * DKR v77/v80 and JFG have no exact donor or matching damping/threshold
 * source shape for this exact two-axis motion update.
 */
static void overlay89Update(Overlay89Object *position, Overlay89EffectState *motion,
                            s32 updateRate) {
    if (motion->motionFlags & 1) {
        if (motion->enabled == 0) {
            motion->xVelocity -= motion->xVelocity >> 5;
            if (motion->xVelocity >= -0x20 && motion->xVelocity < 0x21) {
                motion->xVelocity = 0;
            }
        }
        position->angleA += motion->xVelocity * updateRate;
    }

    if (motion->motionFlags & 2) {
        if (motion->enabled == 0) {
            motion->yVelocity -= motion->yVelocity >> 5;
            if (motion->yVelocity >= -0x20 && motion->yVelocity < 0x21) {
                motion->yVelocity = 0;
            }
        }
        position->angleB += motion->yVelocity * updateRate;
    }
}

/* Mickey-derived reconstruction. The scale member keeps the unsigned size
 * conversion ahead of the scale load without a named float carrier. */
void overlay89InitializeEffect(Overlay89Object *object,
                               Overlay89Init *init) {
    Overlay89NestedRoot *root;
    Overlay89CreateDescriptor descriptor;
    Overlay89EffectState *state;
    Overlay89ColorEntry *source;
    Overlay89ColorEntry *colors;
    s32 count;
    f32 range;

    object->angleA = init->angleA << 8;
    state = object->state;
    object->angleB = init->angleB << 8;

    object->size = (f32)(u32)init->size * gOverlay89InitScale.scale;

    state->mode = init->mode;
    state->motionFlags = init->motionFlags;
    state->index = init->index;
    state->particleCount = init->particleCount;
    state->red = init->red;
    state->green = init->green;
    state->blue = init->blue;
    state->intensity = init->intensity;
    state->enabled = 1;
    state->timer = 0;

    range = (f32)(u32)init->squaredRange;
    state->squaredRange = range * range;
    state->xVelocity = init->xVelocity << 6;
    state->yVelocity = init->yVelocity << 6;
    state->left = object->angleA - (init->halfWidth << 7);
    state->right = object->angleA + (init->halfWidth << 7);
    state->top = object->angleB - (init->halfHeight << 7);
    state->bottom = object->angleB + (init->halfHeight << 7);
    state->motionWord = 0;

    state->enabled = overlay89Evaluate(state);
    state->primaryHandle = NULL;
    state->secondaryHandle = NULL;

    descriptor.mode = init->descriptorMode;
    descriptor.kind = 3;
    descriptor.flags = (~init->flags) & 0x61;
    descriptor.sentinelByte = -1;
    descriptor.zero04 = 0;
    descriptor.zero06 = 0;
    descriptor.zero08 = 0;
    descriptor.zero0A = 0;
    descriptor.sizeA = init->range * 8;
    descriptor.sizeB = init->range * 8;
    descriptor.red = init->red;
    descriptor.green = init->green;
    descriptor.blue = init->blue;
    descriptor.intensity = state->enabled ? init->intensity : 0;
    descriptor.sentinel = -1;
    descriptor.zero16 = 0;
    descriptor.zero17 = 0;
    if (init->descriptorMode == 3) {
        descriptor.flags |= 0x40;
    }

    state->primaryHandle =
        addObjectLight(object, &descriptor, state, init);
    state->scale = (f32)(init->scale * 8);
    state->radius = (f32)(init->radius * 8);

    /* Inert region: preserves IDO's caller-saved state allocation. The
     * ordinary nested-if spelling changes its register bank. Cleanup queued. */
    if (1) {
        if (init->descriptorMode == 3 && !(init->flags & 0x40)) {
            overlay89UpdateEffect(object, state);
        }
    }

    object->status->initialized = 1;
    object->active = 0;

    root = *object->nested;
    if (root != NULL) {
        colors = root->colors;
        source = root->header->entries;
        count = root->header->count;
        while (count--) {
            if (source->red == 0 && source->green == 0 && source->blue == 0) {
                colors->red = state->red;
                colors->green = state->green;
                colors->blue = state->blue;
            }
            source++;
            colors++;
        }
    }
}

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
                                      s32 updateRate) {
    f32 randomScale;
    Overlay89Particle particle;
    void *secondaryHandle;
    Overlay89EffectState *state;
    void *primaryHandle;
    s32 count;

    state = object->state;
    primaryHandle = state->primaryHandle;
    secondaryHandle = state->secondaryHandle;
    overlay89Update(object, state, updateRate);

    if (state->enabled == 0) {
        if (state->timer != 0) {
            state->timer -= updateRate;
            if (state->timer <= 0) {
                state->timer = 0;
                object->active = 0;
                if (primaryHandle != NULL) {
                    changeLightIntensity(primaryHandle, state->intensity);
                }
                if (secondaryHandle != NULL) {
                    trackLightColour(secondaryHandle, state->red,
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
                                mathRnd(-0x2000, 0x2000) +
                                object->angleA;
                            particle.angleB =
                                mathRnd(-0x2000, 0x2000) +
                                object->angleB;
                            particle.speed =
                                mathRnd(10, 20) * randomScale;
                            particle.size =
                                mathRnd(12, 25) * randomScale;
                            overlay34CreateRecord(&particle);
                        } while (count--);
                    }
                }
            }
        } else {
            object->active = 1;
            state->timer = mathRnd(30, 480);
            if (primaryHandle != NULL) {
                changeLightIntensity(primaryHandle, 0);
            }
            if (secondaryHandle != NULL) {
                trackLightColour(secondaryHandle, state->red,
                                       state->green, state->blue, 0);
            }
        }
    }

    do { } while (0);
    do { } while (0);
    if (secondaryHandle != NULL) {
        overlay89UpdateEffect(object, state);
    }
    object->clearWord = 0;
}
