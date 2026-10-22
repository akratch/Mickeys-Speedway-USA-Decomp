#include "overlays/overlay_077.h"

/* Declaration order reproduces the overlay's initialized section. */
s32 gOverlay77Handle = 0;
s32 gOverlay77Sequence = 0;
s32 gOverlay77Selection = 0;
void *gOverlay77CallbackArgument = NULL;

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
 *
 * The five constants are literals: their shipped HI16/LO16 pairs are LOCAL
 * records against the module's rodata (+0x10 into its initialized data), the
 * literal pool IDO emits, not named .data globals. Written as literals, with
 * the vertical velocity read from the object at each use, the bytes match.
 */
void overlay77Update(Overlay77Object *object, volatile s32 updateRate) {
    Overlay77State *state;
    volatile f32 unused;
    s32 mode;
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
        moveX = updateRate;
        if (object->velocityY >= 0.0f) {
            deltaY = (-(object->velocityY * object->velocityY)) / -0.2f;
            if (state->targetY <= (object->y.value + deltaY)) {
                state->acceleration = 0.0f;
            } else {
                state->acceleration = 0.4f;
            }
        } else {
            deltaY = (-(object->velocityY * object->velocityY)) / 0.6f;
            if ((object->y.value + deltaY) < state->targetY) {
                state->acceleration = 0.4f;
            } else {
                state->acceleration = 0.0f;
            }
        }

        accel = state->acceleration + -0.1f;
        deltaY = (object->velocityY * moveX) + (((0.5f * accel) * moveX) * moveX);
        object->velocityY = object->velocityY + (accel * moveX);
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
