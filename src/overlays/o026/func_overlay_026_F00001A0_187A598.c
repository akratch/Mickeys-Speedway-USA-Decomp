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

extern u8 D_B18[];

extern void func_overlay_026_F0000D24_187B11CReloc(O26ObjectUpdate *object,
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
extern s32 func_80008128(O26ObjectUpdate *object, f32 arg1, f32 arg2, f32 arg3);
extern void func_8001DCD0(s16 rotation, O26Vec3fUpdate *direction,
                          s16 *pitch, s16 *yaw);
extern void overlay26RecordMinimumReloc(s32 index, f32 value);
extern f32 Powerf(f32 value, s32 exponent);
extern s16 dAngle(s16 first, s16 second, f32 amount);

/* Matched 2026-10-02. Written from the listing in the shape of the matched
 * overlay 29 update: one x/y/z set shared by the records, the heading deltas
 * and the final distance. What closed the last words: func_80008128 takes
 * three floats and is passed plain 0 (three separate immediate loads rather
 * than one shared float zero); the wobble term goes through the call result
 * in x and the s16 delta, whose narrowing assignment orders the conversion
 * before the heading load; and the final distance reads the object's entity
 * before the linked one. */
void func_overlay_026_F00001A0_187A598(O26ObjectUpdate *object,
                                        s32 updateRate) {
    O26StateUpdate *state;
    O26MotionRecord *record;
    O26LinkedUpdate *linked;
    O26EntityUpdate *otherEntity;
    O26TrackUpdate *track;
    f32 radius;
    f32 updateRateF;
    O26Vec3fUpdate oldPosition;
    f32 x;
    f32 y;
    f32 z;
    f32 distance;
    f32 amount;
    s32 i;
    s32 collision;
    O26EntityUpdate *entity;
    s32 step;
    s16 targetHeading;
    s16 delta;
    s16 pitch;
    s16 yaw;
    s32 pad[2];

    state = object->state64;
    radius = 10.0f;
    updateRateF = updateRate;

    if (object->entity48->flags & 2) {
        object->entity48->owner70 = 0;
        object->entity48->flags &= ~2;
    }

    if ((state->recordsActive30 == 0) && (state->pendingEffect31 != 0)) {
        func_overlay_026_F0000D24_187B11CReloc(object, 0x16);
        state->pendingEffect31 = 0;
    }

    if (state->recordsActive30 != 0) {
        record = &state->records[0];
        y = record->velocity.y * updateRateF + -0.1f * updateRateF * updateRateF;
        record->velocity.y += -0.2f * updateRateF;
        x = record->velocity.x * updateRateF;
        z = record->velocity.z * updateRateF;
        record->position.x += x;
        record->position.y += y;
        record->position.z += z;
        record->angleX += record->angularVelocityX * updateRate;
        record->angleY += record->angularVelocityY * updateRate;
        record->angleZ += record->angularVelocityZ * updateRate;
        if ((record->timer != 0) && (record->velocity.y <= 0)) {
            record->timer -= updateRate * 10;
            if (record->timer <= 0) {
                record->timer = 0;
                state->recordsFinished32++;
            }
        }
        record = &state->records[1];
        y = record->velocity.y * updateRateF + -0.1f * updateRateF * updateRateF;
        record->velocity.y += -0.2f * updateRateF;
        x = record->velocity.x * updateRateF;
        z = record->velocity.z * updateRateF;
        record->position.x += x;
        record->position.y += y;
        record->position.z += z;
        record->angleX += record->angularVelocityX * updateRate;
        record->angleY += record->angularVelocityY * updateRate;
        record->angleZ += record->angularVelocityZ * updateRate;
        if ((record->timer != 0) && (record->velocity.y <= 0)) {
            record->timer -= updateRate * 10;
            if (record->timer <= 0) {
                record->timer = 0;
                state->recordsFinished32++;
            }
        }
        record = &state->records[2];
        y = record->velocity.y * updateRateF + -0.1f * updateRateF * updateRateF;
        record->velocity.y += -0.2f * updateRateF;
        x = record->velocity.x * updateRateF;
        z = record->velocity.z * updateRateF;
        record->position.x += x;
        record->position.y += y;
        record->position.z += z;
        record->angleX += record->angularVelocityX * updateRate;
        record->angleY += record->angularVelocityY * updateRate;
        record->angleZ += record->angularVelocityZ * updateRate;
        if ((record->timer != 0) && (record->velocity.y <= 0)) {
            record->timer -= updateRate * 10;
            if (record->timer <= 0) {
                record->timer = 0;
                state->recordsFinished32++;
            }
        }
        record = &state->records[3];
        y = record->velocity.y * updateRateF + -0.1f * updateRateF * updateRateF;
        record->velocity.y += -0.2f * updateRateF;
        x = record->velocity.x * updateRateF;
        z = record->velocity.z * updateRateF;
        record->position.x += x;
        record->position.y += y;
        record->position.z += z;
        record->angleX += record->angularVelocityX * updateRate;
        record->angleY += record->angularVelocityY * updateRate;
        record->angleZ += record->angularVelocityZ * updateRate;
        if ((record->timer != 0) && (record->velocity.y <= 0)) {
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
        x = object->position.x - otherEntity->position.x;
        z = object->position.z - otherEntity->position.z;
        if (sqrtf(x * x + z * z) != 0) {
            targetHeading = Arctanf(x, z);
        } else {
            targetHeading = state->heading2C;
        }

        step = updateRate - 1;
        if (updateRate != 0) {
            do {
                delta = mathDiffAngle(state->heading2C, targetHeading, step) >> 3;
                if (delta > 0x2EE) {
                    delta = 0x2EE;
                }
                if (delta < -0x2EE) {
                    delta = -0x2EE;
                }
                state->heading2C += delta;
            } while (step--);
        }
    }

    object->velocity.x = func_8002A8C0(state->heading2C) * -33.0f;
    object->velocity.z = func_8002A8BC(state->heading2C) * -33.0f;
    object->velocity.y += -1.0f * (f32)updateRate;
    oldPosition.x = object->position.x;
    oldPosition.y = object->position.y;
    oldPosition.z = object->position.z;
    y = object->velocity.y * (f32)updateRate +
        -0.5f * (f32)updateRate * (f32)updateRate;
    object->velocity.y += -1.0f * (f32)updateRate;
    object->position.x += object->velocity.x * (f32)updateRate;
    object->position.y += y;
    object->position.z += object->velocity.z * updateRateF;

    if (state->wobbleActive2E != 0) {
        state->phase20 += ((f32)updateRate / 60.0f) * 10.0f;
        if (6.283185f <= state->phase20) {
            state->phase20 -= 6.283185f;
        }
        x = func_8002A8C0((s32)((state->phase20 / 6.283185f) * 65536.0f));
        delta = (s32)(state->amplitude24 * x);
        object->rotationX = state->heading2C + delta;

        state->amplitude24 -= 150.0f * (f32)updateRate;
        if (state->amplitude24 <= 0) {
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
    i = func_80008128(object, 0, 0, 0);
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
                    (object->position.y - oldPosition.y) / (f32)updateRate;
                func_8001DCD0(object->rotationX, &state->direction08,
                              &pitch, &yaw);
            }
        }

        linked = state->linked04;
        if (linked != NULL) {
            track = linked->track64;
            entity = object->entity48;
            otherEntity = linked->entity48;
            x = entity->position.x - otherEntity->position.x;
            y = entity->position.y - otherEntity->position.y;
            z = entity->position.z - otherEntity->position.z;
            distance = sqrtf(x * x + y * y +
                             z * z);
            overlay26RecordMinimumReloc(track->index, distance);
            if (track->finished3FA != 0) {
                state->linked04 = NULL;
            }
        }
    }

    amount = 1.0f - Powerf(0.9f, (s32)(f32)updateRate);
    object->rotationZ = dAngle(object->rotationZ, pitch, amount);
    amount = 1.0f - Powerf(0.90f, (s32)(f32)updateRate);
    object->rotationY = dAngle(object->rotationY, yaw, amount);
}
