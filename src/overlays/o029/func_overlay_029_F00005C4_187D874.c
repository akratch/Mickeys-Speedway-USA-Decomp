#include "PR/ultratypes.h"

typedef struct Overlay29TailVec3f {
    f32 x;
    f32 y;
    f32 z;
} Overlay29TailVec3f;

typedef struct Overlay29TailRecord {
    s16 angle0;
    s16 angle1;
    s16 angle2;
    u8 pad06[6];
    Overlay29TailVec3f position;
    Overlay29TailVec3f velocity;
    s16 angularVelocity0;
    s16 angularVelocity1;
    s16 angularVelocity2;
    s16 timer;
} Overlay29TailRecord;

typedef struct Overlay29TailEntity {
    u8 pad00[6];
    u16 flags;
    u8 pad08[0x1C];
    Overlay29TailVec3f position;
    u8 pad30[0x40];
    s32 owner;
} Overlay29TailEntity;

typedef struct Overlay29TailDirection {
    f32 x;
    u8 pad04[4];
    f32 z;
    s16 angle;
    u16 selection;
} Overlay29TailDirection;

typedef struct Overlay29TailLinkedState {
    s8 index;
    u8 pad001[0x39B];
    f32 angle;
} Overlay29TailLinkedState;

typedef struct Overlay29TailObject Overlay29TailObject;

typedef struct Overlay29TailState {
    Overlay29TailObject *owner;
    u16 selection0;
    u16 selection1;
    u8 pad08[3];
    s8 recordsActive;
    f32 ratio;
    f32 referenceAngle;
    u32 collisionFlags;
    s16 recordsFinished;
    u8 skipTargeting;
    s8 pendingEffect;
    u8 pad1C[0xC];
    Overlay29TailRecord records[4];
} Overlay29TailState;

struct Overlay29TailObject {
    s16 angle0;
    s16 angle1;
    s16 angularVelocity0;
    s16 flags06;
    f32 scale;
    Overlay29TailVec3f position;
    u8 pad18[4];
    Overlay29TailVec3f velocity;
    u8 pad28[0x20];
    Overlay29TailEntity *entity;
    u8 pad4C[0x18];
    Overlay29TailState *state;
};

typedef struct Overlay29TailLinkedObject {
    u8 pad00[0x48];
    Overlay29TailEntity *entity;
    u8 pad4C[0x18];
    Overlay29TailLinkedState *state;
} Overlay29TailLinkedObject;

extern Overlay29TailDirection *D_4;
extern Overlay29TailDirection *D_8;
extern f32 overlay29AccelerationReloc;
extern f32 overlay29VelocityStepReloc;
extern f32 overlay29FallbackDistanceReloc;
extern f32 D_C;
extern f32 D_10;
extern u8 D_EE0[];

extern void func_overlay_029_F00001C4_187D474(Overlay29TailObject *object);
extern void func_overlay_029_F000023C_187D4EC(Overlay29TailObject *object,
                                               Overlay29TailState *state);
extern void func_overlay_029_F0000304_187D5B4(Overlay29TailState *state,
                                               f32 *x, f32 *y, f32 *z,
                                               f32 advance);
extern void func_overlay_029_F0000084_187D334(s32 count);
extern void func_overlay_029_F0000124_187D3D4(s32 count);
extern void overlay29Select(s32 index);
extern s32 func_overlay_001_F0000758_184CB38(
    Overlay29TailDirection *direction, f32 x, f32 z);
extern Overlay29TailLinkedObject *func_overlay_001_F000280C_184EBEC(f32 angle);
extern void ext_o0_6a50();
extern void ext_o0_3e99c(Overlay29TailObject *object, s32 updateRate);
extern f32 ext_o0_6ec00(f32 value);
extern s16 ext_o0_2a4c0(f32 y, f32 x);
extern s32 ext_o0_2a5bc(s16 current, s16 target);
extern void ext_o0_29b94(Overlay29TailObject *object,
                          Overlay29TailVec3f *vector);
extern void ext_o0_7cd8(Overlay29TailObject *object, f32 x, f32 y, f32 z);
extern void func_800150F0(s32 count, Overlay29TailVec3f *start,
                          Overlay29TailVec3f *end, f32 *radius, void *arg4,
                          s32 arg5);
extern s32 func_800104B0(Overlay29TailVec3f *start,
                         Overlay29TailVec3f *end, f32 radius,
                         Overlay29TailObject *object, void *callback);
extern void func_overlay_037_F00004F4_1885B14(s32 index, f32 distance);

#define UPDATE_RECORD(record_) \
    do { \
        record = (record_); \
        previousVelocity = record->velocity.y; \
        record->velocity.y = previousVelocity + velocityStep; \
        record->angle0 += record->angularVelocity0 * updateRate; \
        record->position.y += \
            (previousVelocity * updateRateF) + acceleration; \
        record->angle1 += record->angularVelocity1 * updateRate; \
        record->position.x += record->velocity.x * updateRateF; \
        record->angle2 += record->angularVelocity2 * updateRate; \
        record->position.z += record->velocity.z * updateRateF; \
        if ((record->timer != 0) && (record->velocity.y <= 0.0f)) { \
            record->timer -= updateRate * 10; \
            if (record->timer <= 0) { \
                record->timer = 0; \
                state->recordsFinished++; \
            } \
        } \
    } while (0)

/* Size/frame closed at 583 words and 0xC8. Masked 374, first +0x8.
 * L99 unused pointer/f32 (frame) and skipTargeting inverted CFG (the missing
 * word). Remaining: updateRateF is coloured f20 at the convert, so f20 is
 * saved before ra; target converts into a temp, stores 0xC4, saves f20 later.
 * Identity-gated instrumented IDO, proc 0. */
#ifdef NON_MATCHING
void func_overlay_029_F00005C4_187D874(Overlay29TailObject *object,
                                        s32 updateRate) {
    f32 updateRateF = (f32)updateRate;
    void *unused;
    void *unused2;
    f32 unusedF;
    Overlay29TailState *state;
    Overlay29TailRecord *record;
    Overlay29TailLinkedObject *linked;
    Overlay29TailEntity *linkedEntity;
    Overlay29TailVec3f oldPosition;
    f32 acceleration;
    f32 velocityStep;
    f32 previousVelocity;
    f32 targetX;
    f32 targetY;
    f32 targetZ;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 horizontalDistance;
    f32 distance;
    f32 radius;
    s32 useLinkedPosition;
    s32 remaining;
    s32 delta;
    s16 targetAngle;

    state = object->state;
    func_overlay_029_F00001C4_187D474(object);

    if (object->entity->flags & 2) {
        object->entity->owner = 0;
        object->entity->flags &= ~2;
    }
    if ((state->recordsActive == 0) && (state->pendingEffect != 0)) {
        ext_o0_6a50(object, 0xE);
        state->pendingEffect = 0;
    }

    if (state->recordsActive != 0) {
        acceleration =
            overlay29AccelerationReloc * updateRateF * updateRateF;
        velocityStep = overlay29VelocityStepReloc * updateRateF;
        UPDATE_RECORD(&state->records[0]);
        UPDATE_RECORD(&state->records[1]);
        UPDATE_RECORD(&state->records[2]);
        UPDATE_RECORD(&state->records[3]);
        if (state->recordsFinished == 4) {
            ext_o0_6a50(object);
        }
        return;
    }

    ext_o0_3e99c(object, updateRate);
    if (state->skipTargeting != 0) {
        linked = NULL;
        useLinkedPosition = 0;
    } else {
        if (func_overlay_001_F0000758_184CB38(
                D_8, object->position.x, object->position.z) != 0) {
            func_overlay_029_F0000084_187D334(1);
            state->selection0 = D_8->selection;
            state->selection1 = D_4->selection;
        } else if (func_overlay_001_F0000758_184CB38(
                       D_4, object->position.x, object->position.z) == 0) {
            func_overlay_029_F0000124_187D3D4(1);
            state->selection0 = D_8->selection;
            state->selection1 = D_8->selection;
        }

        func_overlay_029_F000023C_187D4EC(object, state);
        linked = func_overlay_001_F000280C_184EBEC(state->referenceAngle);
        if (linked == (Overlay29TailLinkedObject *)state->owner) {
            linked = func_overlay_001_F000280C_184EBEC(linked->state->angle);
        }
        if (linked != NULL) {
            linkedEntity = linked->entity;
            dx = object->position.x - linkedEntity->position.x;
            dy = object->position.y - linkedEntity->position.y;
            dz = object->position.z - linkedEntity->position.z;
            distance = ext_o0_6ec00((dx * dx) + (dy * dy) + (dz * dz));
        } else {
            distance = overlay29FallbackDistanceReloc;
        }

        useLinkedPosition = 0;
        if (distance < D_C) {
            targetX = linkedEntity->position.x;
            targetY = linkedEntity->position.y;
            targetZ = linkedEntity->position.z;
            useLinkedPosition = 1;
        } else {
            s32 selection;

            selection = 3;
            if (linked != NULL) {
                selection = linked->state->pad001[0x37D];
            }
            overlay29Select(selection);
            func_overlay_029_F0000304_187D5B4(
                state, &targetX, &targetY, &targetZ, D_10);
            targetY += 130.0f;
        }

        remaining = updateRate - 1;
        if (updateRate != 0) {
            do {
                dx = object->position.x - targetX;
                dz = object->position.z - targetZ;
                if (dz == 0.0f) {
                    targetAngle = object->angle0;
                } else {
                    targetAngle = ext_o0_2a4c0(dx, dz);
                }
                delta = ext_o0_2a5bc(object->angle0, targetAngle) >> 3;
                if ((s16)delta >= 0x385) {
                    delta = 0x384;
                }
                if ((s16)delta < -0x384) {
                    delta = -0x384;
                }
                object->angle0 += delta;
                object->angularVelocity0 = delta * 30;

                dy = object->position.y - targetY;
                horizontalDistance = ext_o0_6ec00((dx * dx) + (dz * dz));
                if (horizontalDistance == 0.0f) {
                    targetAngle = object->angle1;
                } else {
                    targetAngle = -ext_o0_2a4c0(dy, horizontalDistance);
                }
                delta = ext_o0_2a5bc(object->angle1, targetAngle) >> 3;
                if ((s16)delta >= 0x385) {
                    delta = 0x384;
                }
                if ((s16)delta < -0x384) {
                    delta = -0x384;
                }
                object->angle1 += delta;
            } while (remaining-- != 0);
        }
    }

    object->velocity.x = 0.0f;
    object->velocity.y = 0.0f;
    object->velocity.z = -30.0f;
    ext_o0_29b94(object, &object->velocity);
    oldPosition = object->position;
    ext_o0_7cd8(object, object->velocity.x * updateRateF,
                 object->velocity.y * updateRateF,
                 object->velocity.z * updateRateF);

    state->collisionFlags = 0;
    radius = 8.0f;
    func_800150F0(1, &oldPosition, &object->position, &radius, NULL, 0);
    if ((func_800104B0(&oldPosition, &object->position, radius, object,
                       D_EE0) != 0) &&
        (state->collisionFlags & 4)) {
        state->pendingEffect = 1;
    }

    if ((useLinkedPosition != 0) && (linked != NULL)) {
        dx = object->entity->position.x - linked->entity->position.x;
        dy = object->entity->position.y - linked->entity->position.y;
        dz = object->entity->position.z - linked->entity->position.z;
        distance = ext_o0_6ec00((dx * dx) + (dy * dy) + (dz * dz));
        func_overlay_037_F00004F4_1885B14(linked->state->index, distance);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o029/func_overlay_029_F00005C4_187D874/func_overlay_029_F00005C4_187D874.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_029_F00005C4_187D874:start
 * symbol: func_overlay_029_F00005C4_187D874
 * score: 374 differing words
 * frame: 0xC8
 * relocations: 43
 * first-mismatch: +0x8
 * summary: Size and frame closed at 583 words and 0xC8. Masked 374, first +0x8. L99 unused pointer/f32 grew the frame; skipTargeting inverted CFG added the missing word. Remaining: convert of updateRateF takes f20 so f20 is saved early; target converts to a temp and stores 0xC4. Identity-gated proc 0.
 * PLATEAU-HANDOFF:func_overlay_029_F00005C4_187D874:end
 */
