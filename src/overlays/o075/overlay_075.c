#include "overlays/overlay_075.h"

/* The target's floating-point multiply schedule requires -Wab,-r4300_mul. */

void overlay75Init(Overlay75Object *object, Overlay75InitData *init, s32 skip) {
    Overlay75Header *header;
    register Overlay75Model *model;
    register void *modelData;
    Overlay75State *state;
    Overlay75DisplayData *display;

    header = object->header;
    object->angle = init->angle;
    state = object->state;
    object->scale = (f32) init->scale * gOverlay75Scale * header->scale;
    object->display->xScale = header->xScale * object->scale;
    object->display->yScale = object->header->yScale * object->scale;
    display = object->display;
    display->flags &= ~0x20;
    object->radius = (f32) object->header->radius * object->scale;

    state->progressLimit = (f32) init->height;
    state->slot = init->slot;
    state->cachedX = object->x;
    state->cachedY = object->y;
    state->cachedZ = object->z;
    state->effectHandle = 0;
    state->progress = 0.0f;
    state->baseX = object->x;
    state->baseY = object->y;
    state->baseZ = object->z;
    state->endpointX = state->baseX - (func_8002A8C0(object->angle) * state->progressLimit);
    state->endpointY = state->baseY;
    state->endpointZ = state->baseZ - (func_8002A8BC(object->angle) * state->progressLimit);

    if (state->slot >= 2) {
        state->advanceRate = 20.0f;
        state->retractRate = 16.0f;
    } else {
        state->advanceRate = 10.0f;
        state->retractRate = 8.0f;
    }

    if (skip == 0) {
        model = object->models[object->modelIndex];
        modelData = model->entity;
        func_8005AD64(object, 0, -1, 0.0f);
        func_8005ABA8(object, 0.0, 0.0);
        func_8005AF14(model, modelData, object);
    }

    gOverlay75SlotFlags[state->slot] = 0;
}

/* DKR v77/v80 and JFG scans found no exact moving-object donor. */

#define O75_FLOAT_BITS(value) (*(u32 *)&(value))

typedef struct Overlay75UpdateLocals {
    s32 moved;
    s32 eventId;
    s32 completed;
    f32 previous;
    u32 pad10;
    f32 moveX;
    u32 pad18;
    f32 delta;
    Overlay75Vector *position;
    void *savedEntity;
    Overlay75Model *model;
} Overlay75UpdateLocals;

/*
 * Exact C: 304 words, the 0x58 frame, and all 20 relocation records.
 *
 * The three work flags are dead on the `cache_position` exit, so they belong
 * *after* the inactive test, not before it. With them in front, the join block
 * after the slot-flag update starts with `moved = 0` and `as1` fills the
 * branch-likely delay slot with that store; the target instead starts the join
 * block with the `active` load, which is why it reads that field twice --
 * once in the annulled slot and once on the fall-through. Moving the flags
 * behind the test hands the load to the join block and the duplication follows.
 *
 * Order inside the moved group is load-bearing to 273 words: the saved entity
 * is read before the tick conversion, and the three flags are cleared in
 * reverse declaration order. Only two of the 120 orders of those five
 * statements are exact.
 */
void overlay75UpdateMovingObject(Overlay75Object *object,
                                       s32 updateRate) {
    Overlay75State *state;
    Overlay75UpdateLocals locals;

    object->eventFlags = 0;
    state = object->state;
    locals.model = object->models[object->modelIndex];
    locals.position = &locals.model->header->vector;

    if (gOverlay75SlotFlags[state->slot] != 0) {
        gOverlay75SlotFlags[state->slot] = 0;
        state->active = 1;
    }

    {
        f32 tick;
        f32 moveZ;

        if (state->active == 0) {
            goto cache_position;
        }
        locals.savedEntity = locals.model->entity;
        tick = (f32)updateRate;
        locals.completed = 0;
        locals.eventId = -1;
        locals.moved = 0;

        if (state->phase == 0) {
            f32 limit;

            locals.moved = 1;
            limit = state->progressLimit;
            locals.delta = state->advanceRate * tick;
            state->progress += locals.delta;
            if (limit <= state->progress) {
                locals.moveX = state->endpointX - object->x;
                moveZ = state->endpointZ - object->z;
                state->progress = limit;
                state->phase = 1;
            } else {
                locals.moveX = -(func_8002A8C0(object->angle) * locals.delta);
                moveZ = -(func_8002A8BC(object->angle) * locals.delta);
            }
            func_80008128(object, locals.moveX, 0.0f, moveZ);
        } else if (state->phase == 1) {
            locals.previous = object->transitionValue;
            if (func_8005ABA8(object,
                              state->advanceRate / 1000.0f,
                              tick) != 0) {
                state->phase = 2;
                state->timer = 240;
                locals.completed = 1;
            } else {
                locals.moved = 1;
            }
            if (locals.previous < gOverlay75ThresholdReloc[1] &&
                gOverlay75ThresholdReloc[1] <= object->transitionValue) {
                if (object->status->alternateEvent != 0) {
                    locals.eventId = 0x1BC;
                } else {
                    locals.eventId = 0x1B9;
                }
            }
        } else if (state->phase == 2) {
            if (object->status->alternateEvent != 0) {
                state->timer = 0;
            }
            state->timer -= updateRate;
            if (state->timer <= 0) {
                state->timer = 0;
                state->phase = 3;
            }
        } else if (state->phase == 3) {
            if (func_8005ABA8(object,
                              -(state->retractRate / 1000.0f),
                              tick) != 0) {
                state->phase = 4;
            } else {
                locals.moved = 1;
            }
        } else if (state->phase == 4) {
            locals.delta = state->retractRate * tick;
            state->progress -= locals.delta;
            if (state->progress <= 0.0f) {
                locals.moveX = state->baseX - object->x;
                moveZ = state->baseZ - object->z;
                state->progress = 0.0f;
                state->phase = 0;
                state->active = 0;
            } else {
                locals.moveX = func_8002A8C0(object->angle) * locals.delta;
                moveZ = func_8002A8BC(object->angle) * locals.delta;
                locals.moved = 1;
            }
            func_80008128(object, locals.moveX, 0.0f, moveZ);
        }

        func_8005AF14(locals.model, locals.savedEntity, object);
        lightObject(object, locals.model, object->renderResource,
                      locals.model->table[locals.model->tableIndex]);
        locals.model->field08 = 0;

        if (locals.moved != 0 && state->effectHandle == 0) {
            amSndPlayXYZ(0x1BA, O75_FLOAT_BITS(locals.position->x),
                                       O75_FLOAT_BITS(locals.position->y),
                                       O75_FLOAT_BITS(locals.position->z), 1,
                                       &state->effectHandle);
        } else if (locals.moved != 0 && state->effectHandle != 0) {
            amSndSetXYZ(state->effectHandle,
                          O75_FLOAT_BITS(locals.position->x),
                          O75_FLOAT_BITS(locals.position->y),
                          O75_FLOAT_BITS(locals.position->z));
        } else if (locals.moved == 0 && state->effectHandle != 0) {
            amSndStopXYZ(state->effectHandle);
        }

        if (locals.eventId != -1) {
            overlay75PlayEventReloc(locals.eventId,
                          O75_FLOAT_BITS(locals.position->x),
                          O75_FLOAT_BITS(locals.position->y),
                          O75_FLOAT_BITS(locals.position->z), 4, 0);
        }
        if (locals.completed != 0) {
            camScreenShake(locals.position->x,
                                           locals.position->y,
                                           locals.position->z,
                                           600.0f, 20.0f);
            object->eventFlags |= 1;
            partUpdateTriggers(object, updateRate);
        }
    }

cache_position:
    state->cachedX = locals.position->x;
    state->cachedY = locals.position->y;
    state->cachedZ = locals.position->z;
}

/* DKR v77 object and semantic-source searches found no exact donor. */

void overlay75MarkSlot(s32 slot) {
    if (slot < 5) {
        gOverlay75SlotFlags[slot] = 1;
    }
}
