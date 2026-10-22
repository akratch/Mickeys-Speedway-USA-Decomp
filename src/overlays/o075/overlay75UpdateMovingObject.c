#include "PR/ultratypes.h"

/* DKR v77/v80 and JFG scans found no exact moving-object donor. */

typedef struct Overlay75Vector {
    f32 x;
    f32 y;
    f32 z;
} Overlay75Vector;

typedef struct Overlay75Header {
    u8 pad00[0x0C];
    Overlay75Vector vector;
} Overlay75Header;

typedef struct Overlay75Model {
    void *entity00;
    u8 pad04[4];
    s16 field08;
    s16 tableIndex0A;
    void *table0C[13];
    Overlay75Header *header40;
} Overlay75Model;

typedef struct Overlay75State {
    s16 slot00;
    s16 active02;
    s16 phase04;
    s16 timer06;
    f32 advanceRate08;
    f32 retractRate0C;
    f32 baseX10;
    f32 baseY14;
    f32 baseZ18;
    f32 endpointX1C;
    f32 endpointY20;
    f32 endpointZ24;
    f32 progress28;
    f32 progressLimit2C;
    f32 cachedX30;
    f32 cachedY34;
    f32 cachedZ38;
    void *effectHandle3C;
} Overlay75State;

typedef struct Overlay75Status {
    u8 pad00[0x61];
    u8 alternateEvent61;
} Overlay75Status;

typedef struct Overlay75Object {
    s16 angle00;
    u8 pad02[0x0A];
    f32 x0C;
    f32 y10;
    f32 z14;
    u8 pad18[0x10];
    f32 transitionValue28;
    u8 pad2C[0x0E];
    s8 modelIndex3A;
    u8 pad3B[0x0D];
    Overlay75Status *status48;
    u8 pad4C[4];
    void *renderResource50;
    u8 pad54[0x10];
    Overlay75State *state64;
    Overlay75Model **models68;
    u8 pad6C[0x14];
    u32 eventFlags80;
} Overlay75Object;

extern s32 gOverlay75SlotFlags[];
extern f32 gOverlay75ThresholdReloc[];
extern f32 func_8002A8C0(s16 angle);
extern f32 func_8002A8BC(s16 angle);
extern void func_80008128(Overlay75Object *object, f32 x, f32 y, f32 z);
extern s32 func_8005ABA8(Overlay75Object *object, f32 rate, f32 tick);
extern void func_8005AF14(Overlay75Model *model, void *entity,
                                       Overlay75Object *object);
extern void lightObject(Overlay75Object *object, Overlay75Model *model,
                                    void *renderResource, void *tableEntry);
extern void amSndPlayXYZ(s32 id, u32 x, u32 y, u32 z,
                                       s32 mode, void **output);
extern void camScreenShake(f32 x, f32 y, f32 z, f32 scale,
                                          f32 lifetime);
extern void amSndSetXYZ(void *handle, u32 x, u32 y, u32 z);
extern void amSndStopXYZ(void *handle);
extern void overlay75PlayEventReloc(u16 eventId, u32 x, u32 y, u32 z, s32 mode,
                                    s32 flags);
extern void partUpdateTriggers(Overlay75Object *object, s32 updateRate);

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
 * block with the `active02` load, which is why it reads that field twice --
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

    object->eventFlags80 = 0;
    state = object->state64;
    locals.model = object->models68[object->modelIndex3A];
    locals.position = &locals.model->header40->vector;

    if (gOverlay75SlotFlags[state->slot00] != 0) {
        gOverlay75SlotFlags[state->slot00] = 0;
        state->active02 = 1;
    }

    {
        f32 tick;
        f32 moveZ;

        if (state->active02 == 0) {
            goto cache_position;
        }
        locals.savedEntity = locals.model->entity00;
        tick = (f32)updateRate;
        locals.completed = 0;
        locals.eventId = -1;
        locals.moved = 0;

        if (state->phase04 == 0) {
            f32 limit;

            locals.moved = 1;
            limit = state->progressLimit2C;
            locals.delta = state->advanceRate08 * tick;
            state->progress28 += locals.delta;
            if (limit <= state->progress28) {
                locals.moveX = state->endpointX1C - object->x0C;
                moveZ = state->endpointZ24 - object->z14;
                state->progress28 = limit;
                state->phase04 = 1;
            } else {
                locals.moveX = -(func_8002A8C0(object->angle00) * locals.delta);
                moveZ = -(func_8002A8BC(object->angle00) * locals.delta);
            }
            func_80008128(object, locals.moveX, 0.0f, moveZ);
        } else if (state->phase04 == 1) {
            locals.previous = object->transitionValue28;
            if (func_8005ABA8(object,
                              state->advanceRate08 / 1000.0f,
                              tick) != 0) {
                state->phase04 = 2;
                state->timer06 = 240;
                locals.completed = 1;
            } else {
                locals.moved = 1;
            }
            if (locals.previous < gOverlay75ThresholdReloc[1] &&
                gOverlay75ThresholdReloc[1] <= object->transitionValue28) {
                if (object->status48->alternateEvent61 != 0) {
                    locals.eventId = 0x1BC;
                } else {
                    locals.eventId = 0x1B9;
                }
            }
        } else if (state->phase04 == 2) {
            if (object->status48->alternateEvent61 != 0) {
                state->timer06 = 0;
            }
            state->timer06 -= updateRate;
            if (state->timer06 <= 0) {
                state->timer06 = 0;
                state->phase04 = 3;
            }
        } else if (state->phase04 == 3) {
            if (func_8005ABA8(object,
                              -(state->retractRate0C / 1000.0f),
                              tick) != 0) {
                state->phase04 = 4;
            } else {
                locals.moved = 1;
            }
        } else if (state->phase04 == 4) {
            locals.delta = state->retractRate0C * tick;
            state->progress28 -= locals.delta;
            if (state->progress28 <= 0.0f) {
                locals.moveX = state->baseX10 - object->x0C;
                moveZ = state->baseZ18 - object->z14;
                state->progress28 = 0.0f;
                state->phase04 = 0;
                state->active02 = 0;
            } else {
                locals.moveX = func_8002A8C0(object->angle00) * locals.delta;
                moveZ = func_8002A8BC(object->angle00) * locals.delta;
                locals.moved = 1;
            }
            func_80008128(object, locals.moveX, 0.0f, moveZ);
        }

        func_8005AF14(locals.model, locals.savedEntity, object);
        lightObject(object, locals.model, object->renderResource50,
                      locals.model->table0C[locals.model->tableIndex0A]);
        locals.model->field08 = 0;

        if (locals.moved != 0 && state->effectHandle3C == 0) {
            amSndPlayXYZ(0x1BA, O75_FLOAT_BITS(locals.position->x),
                                       O75_FLOAT_BITS(locals.position->y),
                                       O75_FLOAT_BITS(locals.position->z), 1,
                                       &state->effectHandle3C);
        } else if (locals.moved != 0 && state->effectHandle3C != 0) {
            amSndSetXYZ(state->effectHandle3C,
                          O75_FLOAT_BITS(locals.position->x),
                          O75_FLOAT_BITS(locals.position->y),
                          O75_FLOAT_BITS(locals.position->z));
        } else if (locals.moved == 0 && state->effectHandle3C != 0) {
            amSndStopXYZ(state->effectHandle3C);
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
            object->eventFlags80 |= 1;
            partUpdateTriggers(object, updateRate);
        }
    }

cache_position:
    state->cachedX30 = locals.position->x;
    state->cachedY34 = locals.position->y;
    state->cachedZ38 = locals.position->z;
}
