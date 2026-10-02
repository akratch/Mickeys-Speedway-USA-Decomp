#include "PR/ultratypes.h"

typedef struct Overlay87MotionState {
    f32 liftOffset;
    f32 value04;
    f32 radiusSquared;
    f32 x;
    f32 y;
    f32 z;
    f32 verticalVelocity;
    f32 targetHeight;
    f32 verticalAcceleration;
    f32 bounceAcceleration;
    s16 turning;
    s16 phase;
    s16 angle;
    s16 targetAngle;
    u8 pad30[2];
    s16 angularVelocity;
    s16 angularAcceleration;
} Overlay87MotionState;

typedef struct Overlay87MotionObject {
    s16 angle;
    s16 turnAmount;
    s16 liftAmount;
    u8 pad06[6];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[8];
    f32 verticalVelocity;
    u8 pad24[0x17];
    s8 updateMode;
    u8 pad3C[0x28];
    Overlay87MotionState *state;
} Overlay87MotionObject;

extern void *gOverlay87Current;
extern void *gOverlay87UpdateTable[];
extern f32 gOverlay87TurnFactor;
extern f32 gOverlay87StraightFactor;
extern f32 gOverlay87ApproachFactor;
extern f32 gOverlay87PositiveDivisor;
extern f32 gOverlay87PositiveAcceleration;
extern f32 gOverlay87NegativeDivisor;
extern f32 gOverlay87NegativeAcceleration;
extern f32 gOverlay87Gravity;

extern s32 overlay87HasNearby(void *unused, void *query);
extern void func_80002FE0(s32 id, f32 x, f32 y, f32 z, s32 priority,
                          void **handle);
extern f32 Powerf(f32 value, s32 exponent);
extern s16 dAngle(s16 current, s16 target, f32 fraction);
extern s32 Arctanf(f32 y, f32 x);
extern f32 sqrtf(f32 value);
extern f32 func_8002A8C0(s16 angle);
extern f32 func_8002A8BC(s16 angle);
extern s32 func_80008128(Overlay87MotionObject *object, f32 x, f32 y, f32 z);
extern s32 func_8005ABA8(Overlay87MotionObject *object, void *update,
                         f32 updateRate);
extern void func_8005AD64(Overlay87MotionObject *object, s32 mode, s32 index,
                          f32 value);

/* Matched: f32 0 passed as the integer literal 0, func_80008128 returning s32
 * (as its other callers declare it), verticalDelta reused as the divisor term
 * in the lift test, dx reused for the X step, newAngle/crossed reused for the
 * turn delta, and two unused s32 locals that place the frame homes. */
void func_overlay_087_F0000128_18D3090(Overlay87MotionObject *object,
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
            func_80002FE0(0x1BD, state->x, state->y, state->z, 4,
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
