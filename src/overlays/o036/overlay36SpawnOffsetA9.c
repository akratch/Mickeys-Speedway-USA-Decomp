#include "PR/ultratypes.h"

typedef struct Vec3f { f32 x, y, z; } Vec3f;
typedef struct Overlay36Action { u8 pad00[0x20]; u16 kind; } Overlay36Action;
typedef struct Overlay36State {
    u8 pad000[0xA8]; void *resource; u8 pad0AC[0xEE];
    u8 state; u8 countdown; u8 pad19C[4]; Overlay36Action *action;
} Overlay36State;
typedef struct Overlay36Object {
    s16 id0; s16 id2; u8 pad04[8]; f32 x, y, z;
    u8 pad18[0x4C]; Overlay36State *state;
} Overlay36Object;
typedef struct Overlay36Request1214 {
    s16 kind; s16 pad02; s16 x; s16 y; s16 z;
    s16 id0; s16 id2; s16 pad0E; Overlay36Object *owner;
} Overlay36Request1214;
typedef struct Overlay36Locals1214 {
    s32 actionKind;
    f32 offset[3];
    Overlay36Request1214 request;
} Overlay36Locals1214;
typedef struct Overlay36SpawnedObject {
    u8 pad00[0x3C]; s32 state; u8 pad40[0x51]; u8 flag91;
} Overlay36SpawnedObject;

extern void mathOneFloatRPY(Overlay36Object *object, f32 *offset);
extern Overlay36SpawnedObject *func_8000590C(
    Overlay36Request1214 *request, s32 mode);
extern void amSndStopXYZ(void *resource);
extern void amSndPlayXYZ(u16 kind, f32 x, f32 y, f32 z,
                                         s32 mode, void **resource);

void overlay36SpawnOffsetA9(Overlay36Object *object) {
    Overlay36SpawnedObject *spawned;
    Overlay36State *state;
    Overlay36Locals1214 locals;

    state = object->state;
    locals.offset[0] = 0.0f;
    locals.offset[1] = 25.0f;
    locals.offset[2] = 14.0f;
    mathOneFloatRPY(object, locals.offset);
    locals.request.kind = 0xA9;
    locals.request.x = (s16)(locals.offset[0] + object->x);
    locals.request.y = (s16)(locals.offset[1] + object->y);
    locals.request.z = (s16)(locals.offset[2] + object->z);
    locals.request.id0 = object->id0;
    locals.request.id2 = object->id2;
    locals.request.owner = object;
    locals.actionKind = state->action->kind;
    spawned = func_8000590C(&locals.request, 1);
    if (spawned != 0) {
        spawned->state = 0;
        if (spawned->flag91 != 0) {
            locals.actionKind = 0xD;
        }
    }
    if (state->action->kind != 0) {
        if (state->resource != 0) {
            amSndStopXYZ(state->resource);
        }
        amSndPlayXYZ((u16)locals.actionKind, object->x,
                                     object->y, object->z, 4,
                                     &state->resource);
    }
    state->countdown--;
    if (state->countdown == 0) {
        state->state = 0xFF;
        state->action = 0;
    }
}
