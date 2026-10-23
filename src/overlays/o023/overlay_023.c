#include "overlays/overlay_023.h"

/*
 * Overlay 23: one translation unit in ROM order (ADR 0006); all four
 * functions are ORT exports. Exact DKR and JFG scans are negative.
 */

#define OVERLAY23_PIPE_SYNC(packet)                \
    {                                              \
        Gfx *macroCommand = (Gfx *)(packet);       \
        macroCommand->w0 = 0xE7000000;             \
        macroCommand->w1 = 0;                      \
    }

#define OVERLAY23_ENV_WHITE(packet)                \
    {                                              \
        Gfx *macroCommand = (Gfx *)(packet);       \
        macroCommand->w0 = 0xFB000000;             \
        macroCommand->w1 = 0xFFFFFF00;             \
    }

/* The overlay relocation stream resolves these two call sites independently,
 * although the relocatable object stores the same zero-valued proxy symbol. */
extern void overlay23CallReloc();

void overlay23SpawnAttachments(Overlay23OwnerObject *object,
                               Overlay23OwnerAnimation *animation) {
    Overlay23Attachment *attachment;
    Overlay23OwnerPoint *point;
    u8 *resourceEntry;
    Overlay23PointModel *pointModel;
    void *spawned;
    Overlay23SpawnPacket packet;
    s32 index;

    pointModel = object->pointModelLink->current;
    if (animation->blocked != 0 || pointModel->enabled == 0) {
        return;
    }

    attachment = object->attachments;
    resourceEntry = object->model->resourceEntries;
    packet.kind = 0xAA;
    packet.lifetime = 0x78;

    for (index = 0; index < object->attachmentCount && index < 4; index++) {
        point = &pointModel->points[attachment->pointIndex];
        packet.pointX = point->x;
        packet.pointY = point->y;
        packet.pointZ = point->z;
        packet.angle0 = object->angle0;
        packet.angle1 = object->angle1;
        packet.angle2 = object->angle2;
        packet.ownerScale = object->scale;
        packet.scaledSize = attachment->scale * (1.0f / object->model->scale);
        packet.animation0 = animation->angleTable0[index];
        packet.animation1 = animation->angleTable1[index];
        packet.velocityX = (point->x - object->baseX) / 20.0f;
        packet.velocityY = (f32)mathRnd(4, 8);
        packet.velocityZ = (point->z - object->baseZ) / 20.0f;
        packet.motionX = object->motionX;
        packet.motionZ = object->motionZ;
        packet.resource = *(u16 *)resourceEntry;

        spawned = func_8000590C(&packet, 1);
        if (spawned != NULL) {
            *(s32 *)((u8 *)spawned + 0x3C) = 0;
        }

        attachment++;
        resourceEntry += 0xC;
    }
}

/* Effect initializer. */
void overlay23Init(Overlay23Object *object, Overlay23InitData *init) {
    Overlay23State *state;
    Overlay23Transform *transform;
    f32 combinedScale;

    state = object->state;
    state->velocityX = init->velocityX;
    state->velocityY = init->velocityY;
    state->scale = init->stateScale;
    state->timer = init->timer;
    state->halfTimer = init->timer >> 1;
    state->angle0 = init->stateAngle0;
    state->angle1 = init->stateAngle1;
    state->randomStep = mathRnd(0x20, 0x40);
    state->alpha = 255.0f;
    state->randomStep *= 8;
    state->alphaStep = 255.0f / (f32)state->halfTimer;
    state->asset = texLoadSprite(init->assetId, 1);
    if (mathRnd(0, 99) < 50) {
        state->randomStep = -state->randomStep;
    }

    object->angle0 = init->angle0;
    object->angle1 = init->angle1;
    object->angle2 = init->angle2;
    object->x = init->x;
    object->y = init->y;
    object->z = init->z;
    object->scale = init->objectScale;
    transform = object->transform;
    combinedScale = object->scale * state->scale;
    transform->scale *= combinedScale;
    transform = object->transform;
    transform->flags &= ~2;
}

/* Per-frame effect update. */
void overlay23Update(Overlay23Object *object, s32 updateRate) {
    Overlay23State *state;
    f32 damping;
    f32 updateRateF;
    s16 timer;

    state = object->state;
    state->angle0 = state->angle0 + (state->randomStep * updateRate);
    state->timer = state->timer - updateRate;
    damping = Powerf(gOverlay23VelocityFactor);
    state->velocityX = state->velocityX * damping;
    state->velocityY = state->velocityY * damping;
    updateRateF = (f32)updateRate;
    object->transform->velocityX = state->velocityX * updateRateF;
    object->transform->velocityY = state->velocityY * updateRateF;

    timer = state->timer;
    if (timer <= 0) {
        state->alpha = 0.0f;
        func_80006EA0(object, updateRate);
        return;
    }
    if (timer < state->halfTimer) {
        state->alpha -= state->alphaStep * updateRateF;
        if (state->alpha < 0.0f) {
            state->alpha = 0.0f;
        }
        object->alpha = state->alpha;
    }
}

/* Block-local command temporaries reproduce the original display-list macro
 * expansion and its live ranges. */
void overlay23RenderEffect(Overlay23RenderObject *object, Gfx **displayList,
                           void *renderArg0, void *renderArg1) {
    Overlay23RenderState *state;
    Overlay23RenderPacket packet;

    state = object->state;
    overlay23CallReloc();

    packet.mode = 3;
    packet.color = 0x3333;
    packet.value0 = state->value0;
    packet.value1 = state->value1;
    packet.scale = state->scale;
    packet.unitScale = 1.0f;
    packet.x = object->x;
    packet.y = object->y;
    packet.z = object->z;
    packet.resource = state->resource;

    OVERLAY23_PIPE_SYNC((*displayList)++);
    OVERLAY23_ENV_WHITE((*displayList)++);

    overlay23CallReloc(displayList, renderArg0, renderArg1, object,
                       object->renderResource, &packet, 14, object->alpha);
}
