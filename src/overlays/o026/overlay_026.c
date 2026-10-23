#include "overlays/overlay_026.h"

/*
 * Overlay 26: one translation unit in ROM order (ADR 0006). The ORT exports
 * the initializer (+0x0), the update (+0x1A0), the effect handler (+0xD24)
 * and the group renderer (+0x1158). The plane projection (+0xB18) is reached
 * only through its stored address, so it stays extern: IDO drops a static
 * function nothing in the TU names.
 */

void func_overlay_026_F0000000_187A3F8(O26Object *object, O26Config *config) {
    O26State *state;
    f32 radius;
    O26Vec3f position;
    O26Vec3f motion;

    state = object->state;
    radius = 10.0f;
    object->entity->owner70 = config->word10;
    object->entity->flags |= 2;
    object->angleX = config->angleX;
    object->angleY = config->angleY;
    object->angleZ = config->angleZ;
    object->active80 = 1;
    state->short2C = (u16)config->angleX;
    state->word00 = config->word10;
    state->word04 = config->word14;
    state->short2E = 1;
    state->value24 = 12288.0f;

    position.x = object->position.x;
    position.y = object->position.y;
    position.z = object->position.z;
    motion.x = 0.0f;
    motion.y = 0.0f;
    motion.z = -38.0f;
    mathOneFloatRPY(object, &motion);
    object->position.x += motion.x;
    object->position.y += motion.y;
    object->position.z += motion.z;
    trackMakePolylist(1, &position, &object->position, &radius, 0, 0);

    if ((func_80010900(&position, &object->position, radius, object, func_overlay_026_F0000B18_o026Reloc) != 0) &&
        ((state->flags28 & 4) != 0)) {
        func_overlay_026_F0000D24_187B11CReloc(object, 5);
    } else {
        object->direction.z = -33.0f;
        mathOneFloatPY(object, &object->direction);
        object->result2E = func_8000FAE0(object->position.x,
                                        object->position.y,
                                        object->position.z);
    }
}

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
                              object, func_overlay_026_F0000B18_o026Reloc);
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
            overlay37RecordMinimum(track->index, distance);
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

/* `lengthSquared` carries the plane offset after the normalisation divides:
 * one fewer declared local leaves a word below the local block, which is what
 * puts the caller-save spill of the first component at sp+0x44. */
void func_overlay_026_F0000B18_187AF10(
    s32 unused, O26ProjectionVec3f *out, O26ProjectionVec3f *direction,
    f32 distance, O26ProjectionPlane *plane, O26ProjectionOwner *owner) {
    O26ProjectionResult *result;
    f32 crossX;
    f32 crossY;
    f32 crossZ;
    f32 planeConstant;
    f32 projectedX;
    f32 projectedY;
    f32 projectedZ;
    f32 lengthSquared;
    f32 normalY = plane->normal.y;
    f32 normalX = plane->normal.x;
    f32 normalZ = plane->normal.z;

    result = owner->result64;
    planeConstant = plane->constant;
    if ((D_14 <= normalY) || (plane->flags & 0x10000000)) {
        crossX = direction->z * normalY;
        crossY = -(direction->z * normalX) +
                 (normalZ * direction->x);
        crossZ = -(direction->x * normalY);

        projectedX = (crossY * normalZ) - (crossZ * normalY);
        projectedY = (crossZ * normalX) - (crossX * normalZ);
        projectedZ = (crossX * normalY) - (crossY * normalX);
        lengthSquared = (projectedX * projectedX) +
                        (projectedY * projectedY) +
                        (projectedZ * projectedZ);

        if (D_18 < lengthSquared) {
            lengthSquared = sqrtf(lengthSquared);
            projectedX /= lengthSquared;
            projectedY /= lengthSquared;
            projectedZ /= lengthSquared;
            lengthSquared = distance - plane->distance;
            out->x = plane->point.x + (lengthSquared * projectedX);
            out->y = plane->point.y + (lengthSquared * projectedY);
            out->z = plane->point.z + (lengthSquared * projectedZ);
        } else {
            out->y = (-((out->z * normalZ) +
                        (normalX * out->x) + planeConstant) /
                      normalY) +
                     D_1C;
        }

        result->normal08.x = normalX;
        result->normal08.y = normalY;
        result->normal08.z = normalZ;
        result->flags28 |= 2;
    } else {
        out->x = plane->point.x;
        out->y = plane->point.y;
        out->z = plane->point.z;
        result->normal14.x = normalX;
        result->normal14.y = normalY;
        result->normal14.z = normalZ;
        result->flags28 |= 4;
    }
}

/* Removing both redundant input aliases recovers the local and temporary
 * stack homes together without changing the effect operations. */
void func_overlay_026_F0000D24_187B11C(O26ObjectD24 *object, s32 mode) {
    O26StateD24 *state;
    O26EffectRecord *effect;
    O26Angles2 angles;
    s16 azimuth;
    s16 elevation;

    state = object->state;
    if (mode & 1) {
        func_80006EA0(object);
    } else if (mode & 2) {
        azimuth = func_8002A910(state->sourceDirection.x,
                               state->sourceDirection.z);
        elevation = func_8002A910(
            sqrtf((state->sourceDirection.z * state->sourceDirection.z) +
                  (state->sourceDirection.x * state->sourceDirection.x)),
            state->sourceDirection.y);

        angles.x = azimuth;
        angles.y = elevation + 0x3000;
        effect = &state->effects[0];
        effect->direction.x = 0.0f;
        effect->direction.y = 0.0f;
        effect->direction.z = -7.0f;
        func_80029FE4(&angles, &effect->direction);

        effect->position.x = object->position.x;
        effect->position.y = object->position.y;
        effect->position.z = object->position.z;
        effect->angleX = object->angleX;
        effect->angleY = object->angleY;
        effect->angleZ = object->angleZ;
        effect->scale = object->scale;
        effect->randomX = func_8002997C(-0x500, 0x500);
        effect->randomY = func_8002997C(-0x500, 0x500);
        effect->randomZ = func_8002997C(-0x500, 0x500);
        effect->alpha = 0xFF;

        angles.y = elevation + 0x2000;
        angles.x = azimuth;
        effect = &state->effects[1];
        effect->direction.x = 0.0f;
        effect->direction.y = 0.0f;
        effect->direction.z = -7.0f;
        func_80029FE4(&angles, &effect->direction);

        effect->position.x = object->position.x;
        effect->position.y = object->position.y;
        effect->position.z = object->position.z;
        effect->angleX = object->angleX;
        effect->angleY = object->angleY;
        effect->angleZ = object->angleZ;
        effect->scale = object->scale;
        effect->randomX = func_8002997C(-0x500, 0x500);
        effect->randomY = func_8002997C(-0x500, 0x500);
        effect->randomZ = func_8002997C(-0x500, 0x500);
        effect->alpha = 0xFF;

        angles.x = azimuth - 0x2000;
        angles.y = elevation + 0x2000;
        effect = &state->effects[2];
        effect->direction.x = 0.0f;
        effect->direction.y = 0.0f;
        effect->direction.z = -7.0f;
        func_80029FE4(&angles, &effect->direction);

        effect->position.x = object->position.x;
        effect->position.y = object->position.y;
        effect->position.z = object->position.z;
        effect->angleX = object->angleX;
        effect->angleY = object->angleY;
        effect->angleZ = object->angleZ;
        effect->scale = object->scale;
        effect->randomX = func_8002997C(-0x500, 0x500);
        effect->randomY = func_8002997C(-0x500, 0x500);
        effect->randomZ = func_8002997C(-0x500, 0x500);
        effect->alpha = 0xFF;

        angles.x = azimuth + 0x2000;
        angles.y = elevation + 0x2000;
        effect = &state->effects[3];
        effect->direction.x = 0.0f;
        effect->direction.y = 0.0f;
        effect->direction.z = -7.0f;
        func_80029FE4(&angles, &effect->direction);

        effect->position.x = object->position.x;
        effect->position.y = object->position.y;
        effect->position.z = object->position.z;
        effect->angleX = object->angleX;
        effect->angleY = object->angleY;
        effect->angleZ = object->angleZ;
        effect->scale = object->scale;
        effect->randomX = func_8002997C(-0x500, 0x500);
        effect->randomY = func_8002997C(-0x500, 0x500);
        effect->randomZ = func_8002997C(-0x500, 0x500);
        effect->alpha = 0xFF;
        state->active30 = 1;
    }

    if (mode & 4) {
        object->active80 |= 2;
        func_8003EDEC(object, 1);
    }
    if (mode & 8) {
        amSndPlayXYZ(0x278, object->position.x, object->position.y,
                      object->position.z, 4, 0);
    }
    if (mode & 0x10) {
        amSndPlayXYZ(0x27A, object->position.x, object->position.y,
                      object->position.z, 4, 0);
    }
    object->entity->flags &= ~1;
    object->flags06 |= 0x400;
}

/* PROVENANCE: command expansions use the existing SDK macros in
 * n_audio/mbi.h. Node layout, indexing, call order and command words are
 * reconstructed from Mickey's own overlay. */
void func_overlay_026_F0001158_187B550(Gfx **dl, void *drawContext,
                                       Overlay26Context *context) {
    Overlay26RenderState *render;
    Overlay26Group *group;
    Overlay26Node *node;
    Overlay26ResourceChoice *choice;
    Gfx *gfx;
    u32 resourceSegment;
    s32 groupIndex;

    render = context->render;
    if (render->enabled != 0) {
        gDPPipeSync((*dl)++);
        groupIndex = 4;
        gDPSetEnvColor((*dl)++, 0xFF, 0xFF, 0xFF, 0);

        while (groupIndex--) {
            if (groupIndex == 3) {
                group = &render->group3;
            } else if (groupIndex == 2) {
                group = &render->group2;
            } else {
                if (groupIndex == 1) {
                    group = &render->group1;
                } else {
                    group = &render->group0;
                }
            }

            node = context->nodeTable[4 - groupIndex];
            choice = (Overlay26ResourceChoice *)node->resource;
            if (group->selector == 0xFF) {
                resourceSegment = choice->first;
            } else {
                resourceSegment = choice->alternate;
            }

            lightObject(context, node, context->drawData, 0);
            gDPSetPrimColor((*dl)++, 0, 0, 0xFF, 0xFF, 0xFF, group->selector);
            camPushModelMtx(dl, drawContext, group, 1.0f, 0.0f);

            gfx = (*dl)++;
            gfx->words.w0 = 0xBF000000;
            gfx->words.w1 = node->segment + 0x80000000;
            gSPDisplayList((*dl)++, resourceSegment + 0x80000000);
            gSP1Triangle((*dl)++, 0, 0, 0, 0);
            camRestoreModelMtx(dl);

            node->consumed = 0;
        }

        texDPInit(dl);
        gDPSetPrimColor((*dl)++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
    }
}
