#include "PR/ultratypes.h"

/* The target's floating-point multiply schedule requires -Wab,-r4300_mul. */

typedef struct Overlay75InitData {
    u8 pad0[0xA];
    s16 angle;
    s16 scale;
    s16 slot;
    s16 height;
} Overlay75InitData;

typedef struct Overlay75Header {
    f32 scale;
    u8 pad4[0x14];
    s16 radius;
    u8 pad1A[0x3A];
    f32 xScale;
    f32 yScale;
} Overlay75Header;

typedef struct Overlay75DisplayData {
    f32 xScale;
    f32 yScale;
    u8 pad8[8];
    u8 flags;
} Overlay75DisplayData;

typedef struct Overlay75State {
    s16 slot;
    u8 pad2[6];
    f32 width;
    f32 height;
    f32 x;
    f32 y;
    f32 z;
    f32 projectedX;
    f32 projectedZ;
    f32 projectedY;
    f32 zero;
    f32 velocity;
    f32 baseX;
    f32 baseY;
    f32 baseZ;
    s32 active;
} Overlay75State;

typedef struct Overlay75Object {
    s16 angle;
    u8 pad2[6];
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x1C];
    f32 radius;
    u8 pad38[2];
    s8 modelIndex;
    u8 pad3B[5];
    Overlay75Header *header;
    u8 pad44[8];
    Overlay75DisplayData *display;
    u8 pad50[0x14];
    Overlay75State *state;
    void ***models;
} Overlay75Object;

extern f32 gOverlay75Scale;
extern s32 gOverlay75SlotFlags[];

f32 func_8002A8C0(s16 angle);
f32 func_8002A8BC(s16 angle);
void func_8005AD64(Overlay75Object *object, s32 mode, s32 index,
                        f32 argument);
void func_8005ABA8(Overlay75Object *object, f32 x, f32 y);
void func_8005AF14(void **model, void *data, Overlay75Object *object);

void overlay75Init(Overlay75Object *object, Overlay75InitData *init, s32 skip) {
    Overlay75Header *header;
    register void **model;
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

    state->velocity = (f32) init->height;
    state->slot = init->slot;
    state->baseX = object->x;
    state->baseY = object->y;
    state->baseZ = object->z;
    state->active = 0;
    state->zero = 0.0f;
    state->x = object->x;
    state->y = object->y;
    state->z = object->z;
    state->projectedX = state->x - (func_8002A8C0(object->angle) * state->velocity);
    state->projectedZ = state->y;
    state->projectedY = state->z - (func_8002A8BC(object->angle) * state->velocity);

    if (state->slot >= 2) {
        state->width = 20.0f;
        state->height = 16.0f;
    } else {
        state->width = 10.0f;
        state->height = 8.0f;
    }

    if (skip == 0) {
        model = object->models[object->modelIndex];
        modelData = *model;
        func_8005AD64(object, 0, -1, 0.0f);
        func_8005ABA8(object, 0.0, 0.0);
        func_8005AF14(model, modelData, object);
    }

    gOverlay75SlotFlags[state->slot] = 0;
}
