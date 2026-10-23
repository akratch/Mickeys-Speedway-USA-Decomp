#include "overlays/overlay_087.h"

static s32 overlay87HasNearby(void *unused, Overlay87MotionState *query);

/* DKR v77/v80 and JFG exact-object scans are negative for this initializer. */
void overlay87InitializeObject(Overlay87Object *object,
                               Overlay87Config *config, s32 preserveState) {
    Overlay87MotionState *state;
    f32 value;

    state = object->state;
    object->scale =
        *object->scaleSource * (((void)preserveState, config->scale) *
                          gOverlay87InitScaleReloc);
    state->liftOffset = config->value00;
    state->radiusSquared = config->value08;
    state->phase = 0;
    value = state->radiusSquared;
    state->radiusSquared = value * value;
    state->x = object->x;
    state->y = object->y;
    state->z = object->z;
    state->radius = config->value04;
    state->angle = config->angle;
    state->targetAngle = config->angle;
    state->angularVelocity = 0;
    state->angularAcceleration = 0x300;
    state->targetHeight = object->y;
    object->angle = state->angle + 0x4000;
    func_8005AD64(object, 2, -1, (f32)mathRnd(0, 0x63) / 100.0f);
}

/* Matched: f32 0 passed as the integer literal 0, func_80008128 returning s32
 * (as its other callers declare it), verticalDelta reused as the divisor term
 * in the lift test, dx reused for the X step, newAngle/crossed reused for the
 * turn delta, and two unused s32 locals that place the frame homes. */
void func_overlay_087_F0000128_18D3090(Overlay87Object *object,
                                       s32 updateRate) {
    Overlay87MotionState *state;
    f32 dx;
    f32 verticalDelta;
    f32 dz;
    f32 distance;
    f32 update;
    f32 moveZ;
    s32 newAngle;
    s32 crossed;
    s32 oldAngle;
    s32 unused1;
    s32 nearby;
    s32 unused2;

    update = updateRate;
    state = object->state;
    nearby = overlay87HasNearby(object, state);
    if ((nearby != 0) && (state->phase == 0)) {
        state->phase = 1;
        state->targetHeight = state->y + state->liftOffset;
        func_8005AD64(object, 0, -1, 0);
        if (gOverlay87Current == NULL) {
            amSndPlayXYZ(0x1BD, state->x, state->y, state->z, 4,
                          &gOverlay87Current);
        }
    }

    if (state->phase != 0) {
        oldAngle = state->angle;
        if ((nearby == 0) && (state->phase != 3)) {
            state->phase = 2;
        } else if (nearby != 0) {
            state->phase = 1;
            state->targetHeight = state->y + state->liftOffset;
        }

        if (state->phase == 1) {
            if (state->turning == 0) {
                dx = object->x - state->x;
                dz = object->z - state->z;
                if (state->radiusSquared <= ((dx * dx) + (dz * dz))) {
                    state->targetAngle = Arctanf(dx, dz);
                    state->turning = 1;
                }
            }
            if (state->turning != 0) {
                state->angularVelocity =
                    dAngle(state->angularVelocity, state->angularAcceleration,
                           1.0f - Powerf(gOverlay87TurnFactor, updateRate));
                newAngle = state->angle + (state->angularVelocity * updateRate);
                if ((state->angularVelocity > 0) &&
                    (oldAngle < state->targetAngle) &&
                    (state->targetAngle < newAngle)) {
                    state->turning = 0;
                    state->angularAcceleration = -state->angularAcceleration;
                } else if ((state->angularVelocity < 0) &&
                           (state->targetAngle < oldAngle) &&
                           (newAngle < state->targetAngle)) {
                    state->turning = 0;
                    state->angularAcceleration = -state->angularAcceleration;
                }
                state->angle = newAngle;
            } else {
                state->angularVelocity =
                    dAngle(state->angularVelocity, 0,
                           1.0f - Powerf(gOverlay87StraightFactor, updateRate));
                newAngle = state->angle + (state->angularVelocity * updateRate);
                state->angle = newAngle;
            }
            if (state->verticalVelocity > -10.0f) {
                distance = (state->verticalVelocity * update) +
                           (-0.125f * update * update);
                state->verticalVelocity += -0.25f * update;
                if (state->verticalVelocity < -10.0f) {
                    state->verticalVelocity = -10.0f;
                }
            } else {
                distance = state->verticalVelocity * update;
            }
        } else if (state->phase == 2) {
            dx = object->x - state->x;
            dz = object->z - state->z;
            state->targetAngle = Arctanf(dx, dz);
            state->angularAcceleration = 0x500;
            state->angularVelocity =
                dAngle(state->angularVelocity, state->angularAcceleration,
                       1.0f - Powerf(gOverlay87ApproachFactor, updateRate));
            newAngle = state->angle + (state->angularVelocity * updateRate);
            crossed = FALSE;
            if ((state->angularVelocity > 0) &&
                (oldAngle < state->targetAngle) &&
                (state->targetAngle < newAngle)) {
                crossed = TRUE;
            }
            else if ((state->angularVelocity < 0) &&
                     (state->targetAngle < oldAngle) &&
                     (newAngle < state->targetAngle)) {
                crossed = TRUE;
            }
            if (crossed != FALSE) {
                state->angle = state->targetAngle;
                state->bounceAcceleration =
                    (state->verticalVelocity * state->verticalVelocity) /
                    (2.0f * sqrtf((dx * dx) + (dz * dz)));
                state->angularVelocity = 0;
                state->turning = 0;
                state->angularAcceleration = 0x300;
                state->phase = 3;
                state->targetHeight = state->y;
            } else {
                state->angle = newAngle;
            }
            distance = state->verticalVelocity * update;
        } else if (state->phase == 3) {
            distance = (state->verticalVelocity * update) +
                       (0.5f * state->bounceAcceleration * update * update);
            state->verticalVelocity += state->bounceAcceleration * update;
            if (state->verticalVelocity >= 0.0f) {
                state->verticalVelocity = 0.0f;
                state->bounceAcceleration = 0.0f;
            }
            verticalDelta = object->y - state->y;
            if ((state->verticalVelocity == 0.0f) &&
                (verticalDelta >= -3.0f) && (verticalDelta <= 3.0f)) {
                state->phase = 0;
                func_8005AD64(object, 2, -1, 0);
                object->verticalVelocity = 0.0f;
            }
        }

        if (state->phase != 0) {
            if (object->verticalVelocity >= 0.0f) {
                verticalDelta = -(object->verticalVelocity * object->verticalVelocity) /
                                gOverlay87PositiveDivisor;
                if (state->targetHeight <= object->y + verticalDelta) {
                    state->verticalAcceleration = 0.0f;
                } else {
                    state->verticalAcceleration =
                        gOverlay87PositiveAcceleration;
                }
            } else {
                verticalDelta = -(object->verticalVelocity * object->verticalVelocity) /
                                gOverlay87NegativeDivisor;
                if ((object->y + verticalDelta) < state->targetHeight) {
                    state->verticalAcceleration = gOverlay87NegativeAcceleration;
                } else {
                    state->verticalAcceleration = 0.0f;
                }
            }
            verticalDelta =
                (object->verticalVelocity * update) +
                (0.5f * (state->verticalAcceleration + gOverlay87Gravity) *
                 update * update);
            object->verticalVelocity +=
                (state->verticalAcceleration + gOverlay87Gravity) * update;
        }

        dx = func_8002A8C0(state->angle) * distance;
        moveZ = func_8002A8BC(state->angle) * distance;
        func_80008128(object, dx, verticalDelta, moveZ);
        newAngle = (state->angle - oldAngle) & 0xFFFF;
        crossed = (oldAngle - state->angle) & 0xFFFF;
        if (crossed < newAngle) {
            newAngle = -crossed;
        }
        object->turnAmount = -newAngle * 3;
        object->liftAmount = object->verticalVelocity * 1000.0f;
    }

    if ((func_8005ABA8(object, gOverlay87UpdateTable[object->updateMode],
                       update) != 0) &&
        (object->updateMode == 0)) {
        func_8005AD64(object, 1, -1, 0);
    }
    object->angle = state->angle + 0x4000;
}

/* DKR v77/v80 and JFG checks found only generic null-guarded release logic. */
void overlay87ReleaseCurrent(void) {
    void *resource = gOverlay87Current;

    if (resource != NULL) {
        amSndStopXYZ(resource);
    }
}

/* DKR v77/v80 and JFG exact-object scans are negative for this radius query. */
static s32 overlay87HasNearby(void *unused, Overlay87MotionState *query) {
    Overlay87Object **entries;
    Overlay87Object *entry;
    f32 radiusSquared;
    f32 dx;
    f32 dz;
    u8 result;
    s32 count;

    radiusSquared = query->radius;
    radiusSquared *= radiusSquared;
    entries = func_80005750(&count);
    result = FALSE;
    while (count != 0) {
        entry = *entries++;
        dx = entry->x - query->x;
        dz = entry->z - query->z;
        if ((dx * dx) + (dz * dz) < radiusSquared) {
            result = TRUE;
        }
        count--;
    }
    return result;
}
