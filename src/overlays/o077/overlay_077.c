#include "overlays/overlay_077.h"

/* Declaration order reproduces the overlay's initialized section. */
s32 gOverlay77Handle = 0;
s32 gOverlay77Sequence = 0;
s32 gOverlay77Selection = 0;
void *gOverlay77CallbackArgument = NULL;
f32 gOverlay77PositiveDivisor = -0.2f;
f32 gOverlay77PositiveAcceleration = 0.4f;
f32 gOverlay77NegativeDivisor = 0.6f;
f32 gOverlay77NegativeAcceleration = 0.4f;
f32 gOverlay77Gravity = -0.1f;

/*
 * Overlay 77, ADR 0006 consolidation: overlay77Init.c and overlay77Update.c
 * fold into this one translation unit (overlay77Init at +0x000,
 * overlay77Update at +0x130). The state tail (overlay77EnsureSelection/
 * overlay77RunCallback, +0x3B8) stays a second translation unit,
 * overlay_077_tail.c: both functions here need -Wab,-r4300_mul's R4300
 * multiply-hazard schedule to match, the tail does not, and IDO applies
 * that flag per translation unit, so merging all four functions into one
 * object measurably changes the tail's compiled bytes (a 16-byte .text
 * growth) even though its C is untouched. Two TUs, not one, is the
 * furthest this module can be folded without editing an instruction word.
 */

void overlay77Init(Overlay77Object *object, Overlay77Init *init, s32 preserveSequence) {
    Overlay77State *state;
    f32 radius;

    radius = init->radius & 0xFF;
    state = object->state;
    if (radius < 10.0f) {
        radius = 10.0f;
    }
    radius = radius / 64.0f;
    object->scale = object->header->scale * radius;
    object->field28 = (f32) init->fieldC;
    state->kind = init->kind;
    state->scale = 5.0f;
    state->targetY = object->y.value + 100.0f;
    state->targetX = object->x.value;
    state->targetYCopy = object->y.value;
    state->targetZ = object->z.value;
    object->angle = init->angle;
    object->velocityX = overlay77SinReloc(object->angle) * -20.0f;
    object->velocityZ = overlay77CosReloc(object->angle) * -20.0f;
    if (preserveSequence == 0) {
        state->sequence = gOverlay77Sequence;
        gOverlay77Sequence++;
    }
}

/*
 * Overlay 77 +0x130. The pinned DKR v77/v80 and JFG object ledgers are exact
 * negative. DKR's object initializers and projectile physics are useful
 * semantic references only; no donor name or source was adopted.
 */
void overlay77Update(Overlay77Object *object, volatile s32 updateRate) {
    Overlay77State *state;
    volatile f32 unused;
    s32 mode;
    f32 velY;
    f32 deltaY;
    f32 accel;
    f32 moveX;
    f32 moveZ;
    f32 deltaX;

    state = object->state;
    mode = 9;
    overlay77PathReloc(*object->path, &mode, state->kind, &object->field28,
                       updateRate);

    if (gOverlay77Handle != 0) {
        if (gOverlay77Selection == state->sequence) { if (gOverlay77CallbackArgument == 0) { overlay77SpawnReloc(0x217, object->x.bits, object->y.bits, object->z.bits, 1, &gOverlay77CallbackArgument); } else { overlay77ContinueReloc(gOverlay77CallbackArgument, object->x.bits, object->y.bits, object->z.bits); } } object->flags &= ~0x400;
        velY = object->velocityY;
        accel = 0.0f;
        moveX = updateRate;
        if (velY >= accel) {
            deltaY = (-(velY * velY)) / gOverlay77PositiveDivisor;
            if (state->targetY <= (object->y.value + deltaY)) {
                state->acceleration = accel;
            } else {
                {
                    state->acceleration = gOverlay77PositiveAcceleration;
                }
            }
        } else {
            deltaY = (-(velY * velY)) / gOverlay77NegativeDivisor;
            if ((object->y.value + deltaY) < state->targetY) {
                state->acceleration = gOverlay77NegativeAcceleration;
            } else {
                state->acceleration = accel;
            }
        }

        velY = ((0, object))->velocityY;
        accel = state->acceleration + gOverlay77Gravity;
        deltaY = (velY * moveX) + (((0.5f * accel) * moveX) * moveX);
        object->velocityY = (0, velY) + (accel * moveX);
        accel = 0.0f;
        {
            deltaX = object->velocityX * moveX;
            moveZ = object->velocityZ * moveX;
            if (overlay77MoveReloc(object, deltaX, deltaY, moveZ)) {
                gOverlay77Handle--;
            }
        }
    }

    if (gOverlay77Handle == 0) {
        deltaX = state->targetX - object->x.value;
        deltaY = state->targetYCopy - object->y.value;
        moveZ = state->targetZ - object->z.value;
        overlay77OrientReloc(object, deltaX, deltaY, moveZ);
        object->flags |= 0x400;
        if (gOverlay77CallbackArgument != 0) {
            overlay77StopReloc(gOverlay77CallbackArgument);
        }
    }
}
