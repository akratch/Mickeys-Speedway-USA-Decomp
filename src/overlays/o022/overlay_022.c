#include "overlays/overlay_022.h"

#define FIELD(base, type, off) (*(type *)((u8 *)(base) + (off)))

/*
 * Overlay 22: one translation unit in ROM order (ADR 0006). The ORT exports
 * the initializer (+0x0), the update (+0x2B0) and the remover (+0xD30); the
 * plane resolver (+0xA7C) is reached only through its stored address, so it
 * stays extern: IDO drops a static function nothing in the TU names.
 *
 * The module's .data: the node table and its count, then the literal block
 * the shipped LOCAL pairs address. The count was volatile in the remover's
 * own file; one declaration of the real type matches both readers.
 */
O22Object *gOverlay22Nodes[12] = { 0 };
s32 D_30 = 0;
static u8 overlay22DataPad34[0xC] = { 0 };
static f32 overlay22Constants[5] = { 14.4F, 14.4F, 0.8F, 0.03F, 0.707F };
static u8 overlay22DataTail[0xC] = { 0 };

/* One aggregate, not two pointer locals: cfe copy-propagates a declared
 * pointer whose value is a parameter offset or a symbol address, which leaves
 * both values in pooled cells below the declared block, while an array's
 * elements are not promoted and keep the two homes retail spends at the bottom
 * of the block.  Three details are load-bearing and each was measured: the
 * first call reads the address expression rather than collide[1], because
 * retail uses the register it has just computed; collide[1] is stored
 * immediately before that call, because retail's store sits at the end of the
 * argument setup; and the plane table is read through collide[0] at BOTH
 * calls, or it is rematerialised at the second. */
void func_overlay_022_F0000000_1878108(void *object, void *init) {
    void *contact;
    Vec3f position;
    Vec3f offset;
    f32 distance;
    s32 keep;
    void *collide[2];

    contact = FIELD(object, void *, 0x64);
    FIELD(contact, f32, 4) = (f32)FIELD(init, s32, 0xC);
    FIELD(contact, s8, 0) = (s8)FIELD(init, s16, 0xA);
    FIELD(contact, void *, 0x38) = FIELD(init, void *, 0x14);
    FIELD(contact, s16, 2) = 1;
    position.x = FIELD(object, f32, 0xC);
    position.y = FIELD(object, f32, 0x10);
    position.z = FIELD(object, f32, 0x14);
    offset.x = 0.0f;
    offset.y = 0.0f;
    offset.z = FIELD(init, f32, 0x10);
    mathOneFloatRPY(FIELD(contact, void *, 0x38), &offset);
    FIELD(object, f32, 0xC) += offset.x;
    FIELD(object, f32, 0x10) += offset.y;
    FIELD(object, f32, 0x14) += offset.z;
    keep = 1;
    distance = gOverlay22DistanceReloc;
    collide[1] = &FIELD(object, Vec3f, 0xC);
    trackMakePolylist(1, &position, &FIELD(object, Vec3f, 0xC), &distance, 0, 1);
    collide[0] = func_overlay_022_F0000A7C_o022Reloc;
    if ((func_80010900(&position, collide[1], distance, object, collide[0]) != 0) &&
        (FIELD(contact, u8, 1) & 4)) {
        FIELD(object, s32, 0x80) |= 2;
        partUpdateTriggers(object, 1);
        func_80006EA0(object);
        keep = 0;
    }
    if (keep != 0) {
        if (FIELD(contact, s8, 0) == 2) {
            if (!(FIELD(contact, u8, 1) & 2)) {
                FIELD(contact, u8, 1) = 0;
                position.x = FIELD(object, f32, 0xC);
                position.y = FIELD(object, f32, 0x10) - 5.0f;
                position.z = FIELD(object, f32, 0x14);
                if (func_80010900(collide[1], &position, distance, object,
                                  collide[0]) == 0) {
                    FIELD(contact, s8, 0) = 1;
                    FIELD(object, s16, 4) = 0x4000;
                }
            }
        } else {
            FIELD(object, f32, 0x24) = -FIELD(contact, f32, 4);
            mathOneFloatRPY(FIELD(contact, void *, 0x38),
                          (Vec3f *)((u8 *)object + 0x1C));
            FIELD(object, s16, 4) = 0x4000;
        }
        FIELD(object, s16, 0x2E) = func_8000FAE0(
            FIELD(object, f32, 0xC), FIELD(object, f32, 0x10),
            FIELD(object, f32, 0x14));
        FIELD(FIELD(object, void *, 0x48), void *, 0x70) =
            FIELD(contact, void *, 0x38);
        FIELD(FIELD(object, void *, 0x48), u16, 6) |= 2;
        FIELD(contact, f32, 0x2C) = FIELD(object, f32, 0xC);
        FIELD(contact, f32, 0x30) = FIELD(object, f32, 0x10);
        FIELD(contact, f32, 0x34) = FIELD(object, f32, 0x14);
        FIELD(contact, s16, 2) = 0;
        if (D_30 < 12) {
            gOverlay22Nodes[D_30++] = object;
            return;
        }
        func_overlay_022_F0000D30_1878E38Reloc(gOverlay22Nodes[0], 5, &D_30);
        gOverlay22Nodes[D_30++] = object;
    }
}

/* Matched 2026-10-02 (lane w2-ovlc), rewritten from the listing (402 at -12
 * before). The slope is cross(cross(up, down), up) through the x/y/z locals,
 * which reproduces the up.y spill and the two -1.0 materializations; the
 * same locals carry the position deltas, the normalized bounce velocity and
 * the saved position. accelerationY is the un-negated cosine with
 * -accelerationY in the X/Z products; the reflection reads -dot at each use;
 * vel.z, the slide divide and the friction step read (f32)updateRate. The
 * last 70 words were the frame: the elevation's sqrtf result goes through
 * its own local, because func_8002A8C0(Arctanf(slopeY, sqrtf(...))) makes
 * cfe pool a temp the target does not have (frame 0x98 -> 0x90). */
void func_overlay_022_F00002B0_18783B8(O22Object *object, s32 updateRate) {
    O22State *state;
    f32 updateRateF;
    f32 x;
    f32 y;
    f32 z;
    f32 slopeX;
    f32 slopeY;
    f32 slopeZ;
    f32 distance;
    f32 dot;
    f32 speed;
    f32 length;
    f32 accelerationX;
    f32 accelerationY;
    f32 accelerationZ;
    f32 volume;
    u32 soundVolume;
    s32 collision;
    f32 animationSpeed;
    s32 animationMode;
    s16 angle;

    state = object->state;
    updateRateF = updateRate;
    distance = 14.4f;
    object->flags80 = 0;
    if (object->model->flags06 & 2) {
        object->model->owner70 = 0;
        object->model->flags06 &= ~2;
    }
    if (state->mode == 2) {
        return;
    }
    if ((state->mode == 1) && (state->flags & 2)) {
        x = -(state->up.z * -1.0f);
        y = 0.0f;
        z = state->up.x * -1.0f;
        slopeX = y * state->up.z - z * state->up.y;
        slopeY = z * state->up.x - x * state->up.z;
        slopeZ = state->up.y * x;
        angle = Arctanf(slopeX, slopeZ);
        length = sqrtf(slopeX * slopeX + slopeZ * slopeZ);
        accelerationY = func_8002A8C0(Arctanf(slopeY, length));
        accelerationX = func_8002A8C0(angle) * -accelerationY;
        accelerationZ = func_8002A8BC(angle) * -accelerationY;
    } else {
        accelerationX = 0.0f;
        accelerationZ = 0.0f;
        accelerationY = -1.0f;
    }

    x = object->velocity.x * updateRateF +
        0.5f * accelerationX * updateRateF * updateRateF;
    object->velocity.x += accelerationX * updateRateF;
    y = object->velocity.y * updateRateF +
        0.5f * accelerationY * updateRateF * updateRateF;
    object->velocity.y += accelerationY * updateRateF;
    z = object->velocity.z * updateRateF +
        0.5f * accelerationZ * updateRateF * updateRateF;
    object->velocity.z += accelerationZ * (f32)updateRate;
    object->position.x += x;
    object->position.y += y;
    object->position.z += z;
    state->flags = 0;

    trackMakePolylist(1, &state->previousPosition, &object->position,
                      &distance, 0, 1);
    collision = func_80010900(&state->previousPosition, &object->position,
                              distance, object, func_overlay_022_F0000A7C_o022Reloc);
    if ((func_80008128(object, 0.0f, 0.0f, 0.0f) != 0) ||
        (object->result2E == -1)) {
        object->position.x = state->previousPosition.x;
        object->position.y = state->previousPosition.y;
        object->position.z = state->previousPosition.z;
        func_80008128(object, 0.0f, 0.0f, 0.0f);
        func_overlay_022_F0000D30_1878E38Reloc(object, 5);
        return;
    }

    if (collision & 0x40000000) {
        state->mode = 2;
        state->speed = 0.0f;
    } else if (collision != 0) {
        if (state->flags & 4) {
            speed = object->velocity.x * object->velocity.x +
                    object->velocity.y * object->velocity.y +
                    object->velocity.z * object->velocity.z;
            if (speed > 0.0f) {
                speed = sqrtf(speed);
                x = object->velocity.x / speed;
                y = object->velocity.y / speed;
                z = object->velocity.z / speed;
                if (state->mode == 0) {
                    state->mode = 1;
                }
                speed *= 0.8f;
                dot = x * state->normal.x + y * state->normal.y +
                      z * state->normal.z;
                object->velocity.x = (2.0f * -dot * state->normal.x + x) * speed;
                object->velocity.y = (2.0f * -dot * state->normal.y + y) * speed;
                object->velocity.z = (2.0f * -dot * state->normal.z + z) * speed;
                if (speed > 10.0f) {
                    x = object->position.x;
                    y = object->position.y;
                    z = object->position.z;
                    object->position.x = state->contact.x;
                    object->position.y = state->contact.y;
                    object->position.z = state->contact.z;
                    object->flags80 |= 2;
                    partUpdateTriggers(object, 1);
                    object->position.x = x;
                    object->position.y = y;
                    object->position.z = z;
                }
                soundVolume = func_80001620(0x20B);
                volume = speed * 0.03125f * (f32)soundVolume;
                if ((f32)soundVolume < volume) {
                    volume = (f32)soundVolume;
                }
                if (state->soundHandle != 0) {
                    amSndStopXYZ(state->soundHandle);
                }
                amSndPlayXYZ(0x20B, object->position.x, object->position.y,
                              object->position.z, 4, &state->soundHandle);
                amSndSetVolXYZ(state->soundHandle, (u8)volume);
                state->speed = speed;
            } else {
                state->speed = 0.0f;
                state->mode = 2;
            }
        } else if (state->flags & 2) {
            object->velocity.y = (object->position.y -
                                  state->previousPosition.y) / (f32)updateRate;
            if (state->mode == 1) {
                speed = object->velocity.x * object->velocity.x +
                        object->velocity.y * object->velocity.y +
                        object->velocity.z * object->velocity.z;
                if (speed > 0.0f) {
                    speed = sqrtf(speed);
                    object->velocity.x /= speed;
                    object->velocity.y /= speed;
                    object->velocity.z /= speed;
                    speed -= 0.03f * (f32)updateRate;
                    object->velocity.x *= speed;
                    object->velocity.y *= speed;
                    object->velocity.z *= speed;
                    state->speed = speed;
                }
                if (speed <= 0.0f) {
                    state->speed = 0.0f;
                    state->mode = 2;
                }
            }
        }
    } else {
        state->speed = sqrtf(object->velocity.x * object->velocity.x +
                             object->velocity.y * object->velocity.y +
                             object->velocity.z * object->velocity.z);
    }

    if (state->mode == 0) {
        object->flags80 |= 1;
        partUpdateTriggers(object, updateRate);
    }
    state->previousPosition.x = object->position.x;
    state->previousPosition.y = object->position.y;
    state->previousPosition.z = object->position.z;
    animationSpeed = state->speed;
    if (animationSpeed < 10.0f) {
        animationSpeed = 10.0f;
    }
    animationMode = 9;
    texAnimateSprite(*object->animationEntries, &animationMode,
                  (s32)animationSpeed, object->animationState28, updateRate);
}

/* Reusing crossZ across exclusive branches fixes both call-stack homes.
 * Rotating the two independent cross-product terms reproduces retail FP
 * scheduling; the explicit dereference preserves the remaining operand order. */
void func_overlay_022_F0000A7C_1878B84(
    void *unused, Vec3f *out, Vec3f *direction, f32 distance,
    Overlay22Plane *plane, Overlay22Owner *owner) {
    Overlay22Result *result;
    f32 nx;
    f32 ny;
    f32 nz;
    f32 crossZ;
    f32 projectedX;
    f32 projectedY;
    f32 projectedZ;
    f32 crossX;
    f32 crossY;
    f32 lengthSquared;

    result = owner->result64;
    nx = plane->normal.x;
    ny = plane->normal.y;
    nz = plane->normal.z;

    if ((D_0[4] <= ny) || (((s32)plane->flags << 3) < 0)) {
        crossX = (ny * direction->z) - ((*direction).y * nz);
        crossY = -(direction->z * nx) + (nz * direction->x);
        crossZ = -(direction->x * ny) + (nx * direction->y);

        projectedX = (crossY * nz) - (crossZ * ny);
        projectedY = (crossZ * nx) - (crossX * nz);
        projectedZ = (crossX * ny) - (crossY * nx);
        lengthSquared = (projectedX * projectedX) +
                        (projectedY * projectedY) +
                        (projectedZ * projectedZ);

        if (0.0f < lengthSquared) {
            lengthSquared = sqrtf(lengthSquared);
            projectedY /= lengthSquared;
            projectedX /= lengthSquared;
            projectedZ /= lengthSquared;
            lengthSquared = distance - plane->distance;
            out->x = plane->point.x + (lengthSquared * projectedX);
            out->y = plane->point.y + (lengthSquared * projectedY);
            out->z = plane->point.z + (lengthSquared * projectedZ);
        } else {
            out->x = plane->point.x;
            out->y = plane->point.y;
            out->z = plane->point.z;
        }

        result->normal08.x = nx;
        result->normal08.y = ny;
        result->flags01 |= 2;
        result->normal08.z = nz;
    } else {
        if (result->mode02 != 0) {
            out->x = plane->point.x;
            out->y = plane->point.y;
            out->z = plane->point.z;
        } else {
            f32 dot;

            dot = (direction->z * nz) +
                  ((nx * direction->x) + (ny * direction->y));
            crossZ = -dot;
            crossZ += crossZ;
            projectedX = direction->x + (crossZ * nx);
            projectedY = direction->y + (crossZ * ny);
            projectedZ = direction->z + (crossZ * nz);
            lengthSquared = distance - plane->distance;
            out->x = plane->point.x + (lengthSquared * projectedX);
            out->y = plane->point.y + (lengthSquared * projectedY);
            out->z = plane->point.z + (lengthSquared * projectedZ);

            result->point20.x = plane->point.x;
            result->point20.y = plane->point.y;
            result->point20.z = plane->point.z;
            result->source14.x = nx;
            result->source14.y = ny;
            result->source14.z = nz;
        }
        result->flags01 |= 4;
    }
}

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
/* Matched 2026-09-19 (lane w27-o022), 40 -> 0 masked words at delta 0,
 * frame 0x28, 12 relocations, unforced. A loop-local node carrier makes
 * ugen reuse v0 for the search load (L145). An empty `if (i)` after the
 * compact is an L100 occurrence that drops i's save below the cursor so
 * i takes v1 and occupies v1 through compact; the inert test is tracked
 * in docs/cleanup-queue.md. */
void func_overlay_022_F0000D30_1878E38(O22Object *object, s32 flags) {
    s32 i;
    s32 found;
    s32 count;

    object->model->flags06 &= ~1;
    count = D_30;
    found = -1;
    for (i = 0; i < count; i++) {
        O22Object *node = gOverlay22Nodes[i];
        if (node == object) {
            found = i;
            i = count;
        }
    }

    if (found != -1) {
        while (found < count - 1) {
            gOverlay22Nodes[found] = gOverlay22Nodes[found + 1];
            found++;
        }
        gOverlay22Nodes[count] = 0;
        D_30 = count - 1;
        if (i) {
        }
    }

    if (flags & 1) {
        object->flags80 |= 2;
        partUpdateTriggers(object, 1);
    }
    if (flags & 2) {
        amSndPlayXYZ(0x22B, object->position.x, object->position.y, object->position.z, 4, 0);
    }
    if (flags & 4) {
        amSndPlayXYZ(0x279, object->position.x, object->position.y, object->position.z, 4, 0);
    }
    func_80006EA0(object);
}
