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

extern Overlay29TailDirection *gOverlay29Base1;
extern Overlay29TailDirection *gOverlay29Base2;
extern u8 D_EE0[];

extern void overlay29BuildChain(Overlay29TailObject *object);
extern void overlay29UpdateRatio(Overlay29TailObject *object,
                                 Overlay29TailState *state);
extern void overlay29Sample(Overlay29TailState *state, f32 *x, f32 *y, f32 *z,
                            f32 advance);
extern void overlay29RotateForward(s32 count);
extern void overlay29RotateBackward(s32 count);
extern void overlay29Select(s32 index);
extern void func_overlay_029_F00010C4_187E374Reloc(Overlay29TailObject *object,
                                                   s32 effect);
extern s32 overlay29TestDirectionReloc(Overlay29TailDirection *direction,
                                       f32 x, f32 z);
extern Overlay29TailLinkedObject *overlay29FindPreviousAngleReloc(f32 angle);
extern void overlay29RecordMinimumReloc(s32 index, f32 distance);
extern void func_80006EA0(Overlay29TailObject *object);
extern void partUpdateTriggers(Overlay29TailObject *object, s32 updateRate);
extern f32 sqrtf(f32 value);
extern s32 Arctanf(f32 x, f32 z);
extern s32 mathDiffAngle(s16 current, s16 target);
extern void mathOneFloatPY(Overlay29TailObject *object,
                           Overlay29TailVec3f *vector);
extern s32 func_80008128(Overlay29TailObject *object, f32 x, f32 y, f32 z);
extern void trackMakePolylist(s32 count, Overlay29TailVec3f *start,
                              Overlay29TailVec3f *end, f32 *radius, void *arg4,
                              s32 arg5);
extern s32 func_80010900(Overlay29TailVec3f *start, Overlay29TailVec3f *end,
                         f32 radius, Overlay29TailObject *object,
                         void *callback);

/* Matched 2026-10-02 (lane w2-ovlc), rewritten from the listing (360 before).
 * The four record updates go through x/y/z locals in the order y, vel.y, x,
 * z, the three position adds, then the angles, with the gravity literals
 * inline: the three locals tie on save and take f0/f2/f12 in first-use
 * order, and updateRateF, the acceleration, the step and zero land in
 * f20/f16/f18/f22. Compares against int 0 keep the float zero out of a
 * callee-saved web (no f26). The heading deltas are s16; the selection is an
 * if/else; the final distance reads linkedEntity, linkedState and entity
 * locals, and reusing linkedEntity there is what spills its first range
 * through 0x88. The unused locals are frame cells in the target's order. */
void func_overlay_029_F00005C4_187D874(Overlay29TailObject *object,
                                        s32 updateRate) {
    f32 updateRateF;
    Overlay29TailState *state;
    Overlay29TailRecord *record;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 distance;
    f32 radius;
    Overlay29TailVec3f oldPosition;
    f32 targetX;
    f32 targetY;
    f32 targetZ;
    Overlay29TailLinkedObject *linked;
    Overlay29TailEntity *linkedEntity;
    s32 remaining;
    f32 horizontalDistance;
    s16 targetAngle;
    s16 delta;
    s32 selection;
    s32 i;
    s32 useLinkedPosition;
    f32 x;
    f32 y;
    f32 z;
    Overlay29TailLinkedState *linkedState;
    Overlay29TailEntity *entity;
    s32 unused;

    updateRateF = updateRate;
    state = object->state;
    overlay29BuildChain(object);

    if (object->entity->flags & 2) {
        object->entity->owner = 0;
        object->entity->flags &= ~2;
    }
    if ((state->recordsActive == 0) && (state->pendingEffect != 0)) {
        func_overlay_029_F00010C4_187E374Reloc(object, 0xE);
        state->pendingEffect = 0;
    }

    if (state->recordsActive != 0) {
        record = &state->records[0];
        y = record->velocity.y * updateRateF + -0.1f * updateRateF * updateRateF;
        record->velocity.y += -0.2f * updateRateF;
        x = record->velocity.x * updateRateF;
        z = record->velocity.z * updateRateF;
        record->position.x += x;
        record->position.y += y;
        record->position.z += z;
        record->angle0 += record->angularVelocity0 * updateRate;
        record->angle1 += record->angularVelocity1 * updateRate;
        record->angle2 += record->angularVelocity2 * updateRate;
        if ((record->timer != 0) && (record->velocity.y <= 0)) {
            record->timer -= updateRate * 10;
            if (record->timer <= 0) {
                record->timer = 0;
                state->recordsFinished++;
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
        record->angle0 += record->angularVelocity0 * updateRate;
        record->angle1 += record->angularVelocity1 * updateRate;
        record->angle2 += record->angularVelocity2 * updateRate;
        if ((record->timer != 0) && (record->velocity.y <= 0)) {
            record->timer -= updateRate * 10;
            if (record->timer <= 0) {
                record->timer = 0;
                state->recordsFinished++;
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
        record->angle0 += record->angularVelocity0 * updateRate;
        record->angle1 += record->angularVelocity1 * updateRate;
        record->angle2 += record->angularVelocity2 * updateRate;
        if ((record->timer != 0) && (record->velocity.y <= 0)) {
            record->timer -= updateRate * 10;
            if (record->timer <= 0) {
                record->timer = 0;
                state->recordsFinished++;
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
        record->angle0 += record->angularVelocity0 * updateRate;
        record->angle1 += record->angularVelocity1 * updateRate;
        record->angle2 += record->angularVelocity2 * updateRate;
        if ((record->timer != 0) && (record->velocity.y <= 0)) {
            record->timer -= updateRate * 10;
            if (record->timer <= 0) {
                record->timer = 0;
                state->recordsFinished++;
            }
        }
        if (state->recordsFinished == 4) {
            func_80006EA0(object);
        }
        return;
    }

    partUpdateTriggers(object, updateRate);
    if (state->skipTargeting != 0) {
        linked = NULL;
        useLinkedPosition = 0;
    } else {
        if (overlay29TestDirectionReloc(gOverlay29Base2, object->position.x,
                                              object->position.z) != 0) {
            overlay29RotateForward(1);
            state->selection0 = gOverlay29Base2->selection;
            state->selection1 = gOverlay29Base1->selection;
        } else if (overlay29TestDirectionReloc(
                       gOverlay29Base1, object->position.x, object->position.z) == 0) {
            overlay29RotateBackward(1);
            state->selection0 = gOverlay29Base2->selection;
            state->selection1 = gOverlay29Base2->selection;
        }
        overlay29UpdateRatio(object, state);
        linked = overlay29FindPreviousAngleReloc(state->referenceAngle);
        if (linked == (Overlay29TailLinkedObject *)state->owner) {
            linked = overlay29FindPreviousAngleReloc(linked->state->angle);
        }
        if (linked != NULL) {
            linkedEntity = linked->entity;
            dx = object->position.x - linkedEntity->position.x;
            dy = object->position.y - linkedEntity->position.y;
            dz = object->position.z - linkedEntity->position.z;
            distance = sqrtf(dx * dx + dy * dy + dz * dz);
        } else {
            distance = 3.40282347e+38f;
        }
        useLinkedPosition = 0;
        if (distance < 750.0f) {
            targetX = linkedEntity->position.x;
            targetY = linkedEntity->position.y;
            targetZ = linkedEntity->position.z;
            useLinkedPosition = 1;
        } else {
            if (linked != NULL) {
                selection = linked->state->pad001[0x37D];
            } else {
                selection = 3;
            }
            overlay29Select(selection);
            overlay29Sample(state, &targetX, &targetY,
                                              &targetZ, 0.4f);
            targetY += 130.0f;
        }

        remaining = updateRate - 1;
        if (updateRate != 0) {
            do {
                dx = object->position.x - targetX;
                dz = object->position.z - targetZ;
                if (dz == 0) {
                    targetAngle = object->angle0;
                } else {
                    targetAngle = Arctanf(dx, dz);
                }
                delta = mathDiffAngle(object->angle0, targetAngle) >> 3;
                if (delta > 0x384) {
                    delta = 0x384;
                }
                if (delta < -0x384) {
                    delta = -0x384;
                }
                object->angle0 += delta;
                object->angularVelocity0 = delta * 30;

                dy = object->position.y - targetY;
                horizontalDistance = sqrtf(dx * dx + dz * dz);
                if (horizontalDistance == 0) {
                    targetAngle = object->angle1;
                } else {
                    targetAngle = -Arctanf(dy, horizontalDistance);
                }
                delta = mathDiffAngle(object->angle1, targetAngle) >> 3;
                if (delta > 0x384) {
                    delta = 0x384;
                }
                if (delta < -0x384) {
                    delta = -0x384;
                }
                object->angle1 += delta;
            } while (remaining--);
        }
    }

    object->velocity.x = 0.0f;
    object->velocity.y = 0.0f;
    object->velocity.z = -30.0f;
    mathOneFloatPY(object, &object->velocity);
    oldPosition.x = object->position.x;
    oldPosition.y = object->position.y;
    oldPosition.z = object->position.z;
    func_80008128(object, object->velocity.x * updateRateF,
                  object->velocity.y * updateRateF,
                  object->velocity.z * updateRateF);

    state->collisionFlags = 0;
    radius = 8.0f;
    trackMakePolylist(1, &oldPosition, &object->position, &radius, NULL, 0);
    if ((func_80010900(&oldPosition, &object->position, radius, object,
                       D_EE0) != 0) &&
        (state->collisionFlags & 4)) {
        state->pendingEffect = 1;
    }

    if ((useLinkedPosition != 0) && (linked != NULL)) {
        entity = object->entity;
        linkedEntity = linked->entity;
        linkedState = linked->state;
        dx = entity->position.x - linkedEntity->position.x;
        dy = entity->position.y - linkedEntity->position.y;
        dz = entity->position.z - linkedEntity->position.z;
        distance = sqrtf(dx * dx + dy * dy + dz * dz);
        overlay29RecordMinimumReloc(linkedState->index, distance);
    }
}
