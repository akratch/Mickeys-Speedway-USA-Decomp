#include "PR/ultratypes.h"

typedef struct Vec3f { f32 x, y, z; } Vec3f;
typedef struct Overlay36Action { u8 pad00[0x24]; u16 kind; } Overlay36Action;
typedef struct Overlay36State {
    u8 pad000[4]; f32 direction; u8 pad008[0xA0]; void *resource; u8 pad0AC[0xEE];
    u8 state; u8 countdown; u8 pad19C[4]; Overlay36Action *action;
} Overlay36State;
typedef struct Overlay36Object {
    s16 id0; s16 id2; s16 id4; u8 pad06[6];
    f32 x, y, z; u8 pad18[0x4C]; Overlay36State *state;
} Overlay36Object;
typedef struct Overlay36SpawnRequestF20 {
    s16 kind; s16 pad02; s16 x; s16 y; s16 z; s16 count;
    s32 link; f32 value; Overlay36Object *owner;
} Overlay36SpawnRequestF20;
typedef struct Overlay36SpawnDataF20 {
    f32 offset[3];
    Overlay36SpawnRequestF20 request;
} Overlay36SpawnDataF20;
typedef struct Overlay36SpawnedObject {
    u8 pad00[0x3C]; s32 state; u8 pad40[0x51]; u8 flag91;
} Overlay36SpawnedObject;

extern void mathOneFloatRPY(Overlay36Object *object, f32 *offset);
extern Overlay36SpawnedObject *func_8000590C(
    Overlay36SpawnRequestF20 *request, s32 mode);
extern void amSndStopXYZ(void *resource);
extern void amSndPlayXYZ(u16 kind, f32 x, f32 y, f32 z,
                                         s32 mode, void **resource);

void overlay36SpawnConditional(Overlay36Object *object) {
    Overlay36State *state;
    Overlay36SpawnedObject *spawned;
    Overlay36SpawnDataF20 data;
    s32 actionKind;

    state = object->state;
    data.offset[0] = 0.0f;
    data.offset[1] = 16.0f;
    if (state->direction <= 0.0f) {
        data.offset[2] = 0.0f;
        data.request.value = 45.0f;
    } else {
        data.offset[2] = 14.0f;
        data.request.value = -45.0f;
    }
    mathOneFloatRPY(object, data.offset);
    data.request.kind = 0x9F;
    data.request.x = (s16)(data.offset[0] + object->x);
    data.request.y = (s16)(data.offset[1] + object->y);
    data.request.z = (s16)(data.offset[2] + object->z);
    data.request.count = 2;
    data.request.link = 0;
    data.request.owner = object;
    actionKind = state->action->kind;
    spawned = func_8000590C(&data.request, 1);
    if (spawned != 0) {
        spawned->state = 0;
        if (spawned->flag91 != 0) {
            actionKind = 0xD;
        }
    }
    if (actionKind != 0) {
        if (state->resource != 0) {
            amSndStopXYZ(state->resource);
        }
        amSndPlayXYZ((u16)actionKind, object->x, object->y,
                                     object->z, 4, &state->resource);
    }
    state->countdown--;
    if (state->countdown == 0) {
        state->state = 0xFF;
        state->action = 0;
    }
}
