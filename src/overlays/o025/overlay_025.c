#include "overlays/overlay_025.h"

/* Eight RGB choices; section alignment supplies the initialized tail. */
u8 gOverlay25Colors[8 * 3] = {
    255,   0,   0, /* red */
    255, 128,   0, /* orange */
    255, 255,   0, /* yellow */
      0, 255,   0, /* green */
      0,   0, 255, /* blue */
      0, 255, 255, /* cyan */
    255,   0, 255, /* magenta */
      0,   0,   0, /* black */
};

static void overlay25SetVectorFlags(Overlay25Vector *unused0, Overlay25Vector *out,
                                    Overlay25Vector *unused2, f32 unused3,
                                    Overlay25Source *source,
                                    Overlay25VectorObject *object);

/* Exact 95-word match. Reading the retained owner through state reproduces the
 * shipped s0/s1 carrier allocation; -Wab,-r4300_mul supplies its FP hazard nop. */
void overlay25InitializeEffect(Overlay25Object *object,
                               const Overlay25Init *init) {
    Overlay25InitState *state;
    s32 combinedAngle;
    s32 paletteIndex;

    state = &object->state->init;
    state->lifetime = 600;
    state->duration = 60;
    state->currentValue = 0.0f;

    state->owner = init->owner;

    if (init->useOwner != 0 && state->owner != NULL) {
        Overlay25OwnerState *ownerState;

        ownerState = state->owner->state;
        combinedAngle = ownerState->baseAngle + ownerState->relativeAngle;

        state->activeDuration = 60;
        state->velocityX =
            (func_8002A8C0(combinedAngle) * ownerState->scale) +
            (func_8002A8C0(ownerState->baseAngle) * -32.0f);
        state->lift = 16.0f;
        state->velocityZ =
            (func_8002A8BC(combinedAngle) * ownerState->scale) +
            (func_8002A8BC(ownerState->baseAngle) * -32.0f);
    } else {
        state->activeDuration = 0;
        object->flags |= 0x0800;
    }

    if (D_8007BF1C & 0x10) {
        paletteIndex = func_800299E8(0, 7) * 3;
    } else {
        paletteIndex = 9;
    }
    state->color[0] = gOverlay25Colors[paletteIndex + 0];
    state->color[1] = gOverlay25Colors[paletteIndex + 1];
    state->color[2] = gOverlay25Colors[paletteIndex + 2];

    if (object->vector != NULL) {
        object->vector->x = 0.0f;
        object->vector->y = 0.0f;
    }
}

/* No corresponding DKR/JFG donor was found.
 *
 * Matched 2026-10-02 (lane x-ovla) from a 74-word plateau by rewriting the
 * m2c shape: the movement loop as `steps = updateRate - 1; while (steps--)`
 * over the state fields; `radius = 4`, so the radius takes a ring temporary
 * instead of sharing the else arm's 4.0f constant; the lifetime decrement
 * before the duration one; `while (count--)` over objects[], declared last
 * after the four hit-loop locals whose cells sit above it (`delta` keeps its
 * cell); the height difference carried in `y`, dead in this arm, which gives
 * it y's f2; the disabled test as an early `continue`, one more block inside
 * the loop, which ranks the strength-reduced cursor above the index for s3;
 * and the hit check's result held in `steps` (also dead here) across the
 * `hitSomething = 1` store, which is the shipped delay-slot order. */
void overlay25UpdateEffect(Overlay25Object *object, s32 updateRate) {
    void *unused;
    s32 hitSomething;
    Overlay25EffectState *state;
    f32 x;
    f32 y;
    f32 z;
    s32 steps;
    f32 radius;
    Overlay25Vector position;
    s32 count;
    Overlay25Object *other;
    f32 delta;
    Overlay25EntityState *otherState;
    Overlay25Object *objects[6];

    state = &object->state->effect;
    if (state->activeDuration != 0) {
        state->activeDuration -= updateRate;
        object->value = object->transform->value * 2.0f;
        if (state->activeDuration <= 0) {
            func_80006EA0(object);
            return;
        }
        y = state->lift;
        x = state->velocityX;
        z = state->velocityZ;
        state->lift -= 1.1034483f;
        steps = updateRate - 1;
        while (steps--) {
            x += state->velocityX;
            y += state->lift;
            z += state->velocityZ;
            state->lift -= 1.1034483f;
        }
        position.x = object->x;
        position.y = object->y;
        position.z = *(f32 *)&object->z;
        func_80008128(object, x, y, z);
        radius = 4;
        trackMakePolylist(1, &position, (Overlay25Vector *)&object->x, &radius, 0, 0);
        if (func_80010900(&position, (Overlay25Vector *)&object->x, radius, object,
                                overlay25SetVectorFlags)) {
            if (state->flags & 4) {
                func_80006EA0(object);
                return;
            }
            state->activeDuration = 0;
            object->value = object->transform->value;
            state->multiplier = 0.4f;
            object->flags |= 0x800;
        }
    } else {
        state->lifetime -= updateRate;
        state->duration -= updateRate;
        if (state->duration < 0) {
            state->duration = 0;
        }
        if (state->lifetime <= 0) {
            updateRate = -state->lifetime;
            state->lifetime = 0;
            state->multiplier -= 0.4f * updateRate;
            if (state->multiplier <= 0.1f) {
                func_80006EA0(object);
                return;
            }
        } else {
            state->multiplier += 0.4f * updateRate;
            if (state->multiplier > 4.0f) {
                state->multiplier = 4.0f;
            }
            count = func_8005776C(object->x, object->y, object->z,
                                               object->value * state->multiplier * 16.0f,
                                               1, objects);
            hitSomething = 0;
            while (count--) {
                other = objects[count];
                y = other->y - object->y;
                if ((other != state->owner) || (state->duration == 0)) {
                    otherState = &other->state->entity;
                    if ((otherState->height < -5.0f) && (y > -24.0f) &&
                        (y < 24.0f)) {
                        if (otherState->enabled == 0) {
                            continue;
                        }
                        steps = func_overlay_008_F0000E88_185EBE0(other, otherState);
                        hitSomething = 1;
                        if (steps) {
                            overlay7DispatchModes(state->owner, other);
                            state->owner->state->entity.ownerHitCount++;
                            otherState->selfHitCount++;
                            if (func_80028F54()->type == 5) {
                                func_overlay_070_F0000384_18C954C(other);
                            }
                        }
                    }
                }
            }
            if (hitSomething) {
                state->lifetime = 0;
            }
        }
    }

    if (object->vector != NULL) {
        object->vector->x = state->multiplier * object->transform->scaleX;
        object->vector->y = state->multiplier * object->transform->scaleY;
    }
}

/* No corresponding DKR/JFG source or object match was found.
 *
 * The private callback is retained by the update's actual address reference.
 * Its argument types follow the resident collision routine's callback ABI. */
static void overlay25SetVectorFlags(Overlay25Vector *unused0, Overlay25Vector *out,
                                    Overlay25Vector *unused2, f32 unused3,
                                    Overlay25Source *source,
                                    Overlay25VectorObject *object) {
    Overlay25VectorState *state;

    state = object->state;
    out->x = source->vector.x;
    out->y = source->vector.y;
    out->z = source->vector.z;
    if ((0.707f < source->value) || (source->flags & 0x10000000)) {
        state->flags |= 2;
        return;
    }
    state->flags |= 4;
}
