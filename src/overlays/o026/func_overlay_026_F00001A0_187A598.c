#include "PR/ultratypes.h"

typedef struct O26Vec3fUpdate {
    f32 x;
    f32 y;
    f32 z;
} O26Vec3fUpdate;

typedef struct O26MotionRecord {
    s16 angleX;
    s16 angleY;
    s16 angleZ;
    u8 pad06[6];
    O26Vec3fUpdate position;
    O26Vec3fUpdate velocity;
    s16 angularVelocityX;
    s16 angularVelocityY;
    s16 angularVelocityZ;
    s16 timer;
} O26MotionRecord;

typedef struct O26EntityUpdate {
    u8 pad00[6];
    u16 flags;
    u8 pad08[0x1C];
    O26Vec3fUpdate position;
    u8 pad30[0x40];
    s32 owner70;
} O26EntityUpdate;

typedef struct O26TrackUpdate {
    s8 index;
    u8 pad001[0x3F9];
    s16 finished3FA;
} O26TrackUpdate;

typedef struct O26LinkedUpdate {
    u8 pad00[0x48];
    O26EntityUpdate *entity48;
    u8 pad4C[0x18];
    O26TrackUpdate *track64;
} O26LinkedUpdate;

typedef struct O26StateUpdate {
    s32 word00;
    O26LinkedUpdate *linked04;
    O26Vec3fUpdate direction08;
    u8 pad14[0xC];
    f32 phase20;
    f32 amplitude24;
    s32 collisionFlags28;
    s16 heading2C;
    s16 wobbleActive2E;
    s8 recordsActive30;
    s8 pendingEffect31;
    s16 recordsFinished32;
    O26MotionRecord records[4];
} O26StateUpdate;

typedef struct O26ObjectUpdate {
    s16 rotationX;
    s16 rotationY;
    s16 rotationZ;
    s16 flags06;
    f32 scale08;
    O26Vec3fUpdate position;
    u8 pad18[4];
    O26Vec3fUpdate velocity;
    u8 pad28[6];
    s16 collisionResult2E;
    u8 pad30[0x18];
    O26EntityUpdate *entity48;
    u8 pad4C[0x18];
    O26StateUpdate *state64;
} O26ObjectUpdate;

extern f32 D_0;
extern f32 D_4;
extern f32 D_8;
extern f32 D_C;
extern f32 D_10;
extern u8 D_B18[];

extern void func_overlay_026_F0000D24_187B11C(O26ObjectUpdate *object,
                                               s32 mode);
extern void func_80006EA0(void *object);
extern void partUpdateTriggers(void *object, s32 updateRate);
extern f32 sqrtf(f32 value);
extern s16 Arctanf(f32 x, f32 z);
extern s32 mathDiffAngle(s16 first, s16 second, s32 step);
extern f32 func_8002A8C0(s32 angle);
extern f32 func_8002A8BC(s32 angle);
extern void trackMakePolylist(s32 count, O26Vec3fUpdate *position,
                              O26Vec3fUpdate *origin, f32 *radius,
                              s32 arg4, s32 arg5);
extern s32 func_80010900(O26Vec3fUpdate *position, O26Vec3fUpdate *origin,
                         f32 radius, O26ObjectUpdate *object, void *callback);
extern s32 func_80008128(O26ObjectUpdate *object, s32 arg1, s32 arg2,
                         void *arg3);
extern void func_8001DCD0(s16 rotation, O26Vec3fUpdate *direction,
                          s16 *pitch, s16 *yaw);
extern void overlay37RecordMinimum(s32 index, f32 value);
extern f32 Powerf(f32 value, s32 exponent);
extern s16 dAngle(s16 first, s16 second, f32 amount);

/* Size closed: (s16) casts on both heading-loop compares (sibling o29). 606 words, frame 0x98, 455 masked, first +0x9C. */
/* Named recipe leftover/empty-if/comma/L160/L99/remat is inert or regresses size. Volatile velocityStep is load-bearing. */
/* Remains: 0.0f CSE after record 0 shifts the FP ring (24 windows); D_0/D_4 loads unpaired at +0xAC. Retain GLOBAL_ASM. */
#ifdef NON_MATCHING
void func_overlay_026_F00001A0_187A598(O26ObjectUpdate *object,
                                        s32 updateRate) {
    O26StateUpdate *state;
    O26MotionRecord *record;
    O26LinkedUpdate *linked;
    O26TrackUpdate *track;
    O26EntityUpdate *otherEntity;
    f32 radius;
    f32 updateRateF;
    O26Vec3fUpdate oldPosition;
    f32 acceleration;
    volatile f32 velocityStep;
    f32 previousVelocity;
    f32 deltaX;
    f32 deltaY;
    f32 deltaZ;
    f32 distance;
    f32 amount;
    s32 collision;
    s32 i;
    s32 step;
    s16 targetHeading;
    s16 pitch;
    s16 yaw;

    state = object->state64;
    radius = 10.0f;
    updateRateF = (f32)updateRate;

    if (object->entity48->flags & 2) {
        object->entity48->owner70 = 0;
        object->entity48->flags &= ~2;
    }

    if ((state->recordsActive30 == 0) && (state->pendingEffect31 != 0)) {
        func_overlay_026_F0000D24_187B11C(object, 0x16);
        state->pendingEffect31 = 0;
    }

    if (state->recordsActive30 != 0) {
        record = &state->records[0];
        previousVelocity = record->velocity.y;
        acceleration = D_0 * updateRateF * updateRateF;
        velocityStep = D_4 * updateRateF;
        record->velocity.y = previousVelocity + velocityStep;
        record->position.x += record->velocity.x * updateRateF;
        record->angleX += record->angularVelocityX * updateRate;
        record->position.y += (previousVelocity * updateRateF) + acceleration;
        record->position.z += record->velocity.z * updateRateF;
        record->angleY += record->angularVelocityY * updateRate;
        record->angleZ += record->angularVelocityZ * updateRate;
        if ((record->timer != 0) && (record->velocity.y <= 0.0f)) {
            record->timer -= updateRate * 10;
            if (record->timer <= 0) {
                record->timer = 0;
                state->recordsFinished32++;
            }
        }

        record = &state->records[1];
        previousVelocity = record->velocity.y;
        record->velocity.y = previousVelocity + velocityStep;
        record->position.x += record->velocity.x * updateRateF;
        record->angleX += record->angularVelocityX * updateRate;
        record->position.y += (previousVelocity * updateRateF) + acceleration;
        record->position.z += record->velocity.z * updateRateF;
        record->angleY += record->angularVelocityY * updateRate;
        record->angleZ += record->angularVelocityZ * updateRate;
        if ((record->timer != 0) && (record->velocity.y <= 0.0f)) {
            record->timer -= updateRate * 10;
            if (record->timer <= 0) {
                record->timer = 0;
                state->recordsFinished32++;
            }
        }

        record = &state->records[2];
        previousVelocity = record->velocity.y;
        record->velocity.y = previousVelocity + velocityStep;
        record->position.x += record->velocity.x * updateRateF;
        record->angleX += record->angularVelocityX * updateRate;
        record->position.y += (previousVelocity * updateRateF) + acceleration;
        record->position.z += record->velocity.z * updateRateF;
        record->angleY += record->angularVelocityY * updateRate;
        record->angleZ += record->angularVelocityZ * updateRate;
        if ((record->timer != 0) && (record->velocity.y <= 0.0f)) {
            record->timer -= updateRate * 10;
            if (record->timer <= 0) {
                record->timer = 0;
                state->recordsFinished32++;
            }
        }

        record = &state->records[3];
        previousVelocity = record->velocity.y;
        record->velocity.y = previousVelocity + velocityStep;
        record->position.x += record->velocity.x * updateRateF;
        record->angleX += record->angularVelocityX * updateRate;
        record->position.y += (previousVelocity * updateRateF) + acceleration;
        record->position.z += record->velocity.z * updateRateF;
        record->angleY += record->angularVelocityY * updateRate;
        record->angleZ += record->angularVelocityZ * updateRate;
        if ((record->timer != 0) && (record->velocity.y <= 0.0f)) {
            record->timer -= updateRate * 10;
            if (record->timer <= 0) {
                record->timer = 0;
                state->recordsFinished32++;
            }
        }

        if (state->recordsFinished32 == 4) {
            func_80006EA0(object);
        }
        return;
    }

    partUpdateTriggers(object, updateRate);
    linked = state->linked04;
    if (linked != NULL) {
        otherEntity = linked->entity48;
        deltaX = object->position.x - otherEntity->position.x;
        deltaZ = object->position.z - otherEntity->position.z;
        if (sqrtf((deltaX * deltaX) + (deltaZ * deltaZ)) != 0.0f) {
            targetHeading = Arctanf(deltaX, deltaZ);
        } else {
            targetHeading = state->heading2C;
        }

        step = updateRate - 1;
        if (updateRate != 0) {
            do {
                i = mathDiffAngle(state->heading2C, targetHeading, step) >> 3;
                if ((s16)i >= 0x2EF) {
                    i = 0x2EE;
                }
                if ((s16)i < -0x2EE) {
                    i = -0x2EE;
                }
                state->heading2C += i;
            } while (step--);
        }
    }

    object->velocity.x = func_8002A8C0(state->heading2C) * -33.0f;
    object->velocity.z = func_8002A8BC(state->heading2C) * -33.0f;
    object->velocity.y += -1.0f * updateRateF;
    oldPosition = object->position;
    previousVelocity = object->velocity.y;
    object->velocity.y += -1.0f * updateRateF;
    object->position.x += object->velocity.x * updateRateF;
    object->position.y += (previousVelocity * updateRateF) +
                          (-0.5f * updateRateF * updateRateF);
    object->position.z += object->velocity.z * updateRateF;

    if (state->wobbleActive2E != 0) {
        state->phase20 += (updateRateF / 60.0f) * 10.0f;
        if (D_8 <= state->phase20) {
            state->phase20 -= D_8;
        }
        object->rotationX = state->heading2C +
            (s32)(state->amplitude24 *
                  func_8002A8C0((s32)((state->phase20 / D_8) * 65536.0f)));
        state->amplitude24 -= 150.0f * updateRateF;
        if (state->amplitude24 <= 0.0f) {
            state->wobbleActive2E = 0;
            state->amplitude24 = 0.0f;
        }
    }

    state->collisionFlags28 = 0;
    oldPosition.y += 10.0f;
    object->position.y += 10.0f;
    trackMakePolylist(1, &oldPosition, &object->position, &radius, 0, 0);
    collision = func_80010900(&oldPosition, &object->position, radius,
                              object, D_B18);
    i = func_80008128(object, 0, 0, NULL);
    object->position.y -= 10.0f;
    pitch = 0;
    yaw = 0;
    if ((i != 0) || (object->collisionResult2E == -1)) {
        func_80006EA0(object);
    } else {
        if (collision != 0) {
            if (state->collisionFlags28 & 4) {
                state->pendingEffect31 = 1;
            } else if (state->collisionFlags28 & 2) {
                object->velocity.y =
                    (object->position.y - oldPosition.y) / updateRateF;
                func_8001DCD0(object->rotationX, &state->direction08,
                              &pitch, &yaw);
            }
        }

        linked = state->linked04;
        if (linked != NULL) {
            track = linked->track64;
            otherEntity = linked->entity48;
            deltaX = object->entity48->position.x - otherEntity->position.x;
            deltaY = object->entity48->position.y - otherEntity->position.y;
            deltaZ = object->entity48->position.z - otherEntity->position.z;
            distance = sqrtf((deltaX * deltaX) + (deltaY * deltaY) +
                             (deltaZ * deltaZ));
            overlay37RecordMinimum(track->index, distance);
            if (track->finished3FA != 0) {
                state->linked04 = NULL;
            }
        }
    }

    amount = 1.0f - Powerf(D_C, (s32)updateRateF);
    object->rotationZ = dAngle(object->rotationZ, pitch, amount);
    amount = 1.0f - Powerf(D_10, (s32)updateRateF);
    object->rotationY = dAngle(object->rotationY, yaw, amount);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o026/func_overlay_026_F00001A0_187A598/func_overlay_026_F00001A0_187A598.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_026_F00001A0_187A598:start
 * symbol: func_overlay_026_F00001A0_187A598
 * score: 455/606 words
 * frame: 0x98
 * relocations: 22
 * first-mismatch: +0x9C
 * summary: Size closed by (s16) heading casts (o29). 455/606, frame 0x98, first +0x9C. Recipe leftover/empty-if/comma/L160/L99/remat inert or regress. FP 0.0f CSE remains.
 * PLATEAU-HANDOFF:func_overlay_026_F00001A0_187A598:end
 */
