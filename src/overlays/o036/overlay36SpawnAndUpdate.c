#include "PR/ultratypes.h"

typedef struct Overlay36Action {
    u8 pad00[0x22];
    u16 kind;
} Overlay36Action;

typedef struct Overlay36State {
    u8 pad000[0xA8];
    void *resource;
    u8 pad0AC[0xEE];
    u8 state;
    u8 countdown;
    u8 pad19C[4];
    Overlay36Action *action;
} Overlay36State;

typedef struct Overlay36Object {
    u8 pad00[0x0C];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x4C];
    Overlay36State *state;
} Overlay36Object;

typedef struct Overlay36SpawnRequest {
    s16 kind;
    s16 pad02;
    s16 x;
    s16 y;
    s16 z;
    s16 count;
    Overlay36Object *owner;
} Overlay36SpawnRequest;

typedef struct Overlay36SpawnedObject {
    u8 pad00[0x3C];
    s32 state;
} Overlay36SpawnedObject;

extern Overlay36SpawnedObject *func_8000590C(
    Overlay36SpawnRequest *request, s32 mode);
extern void amSndStopXYZ(void *resource);
extern void amSndPlayXYZ(u16 kind, f32 x, f32 y, f32 z,
                                         s32 mode, void **resource);

void overlay36SpawnAndUpdate(Overlay36Object *object, s32 count) {
    Overlay36SpawnRequest request;
    Overlay36SpawnedObject *spawned;
    Overlay36State *state;

    state = object->state;
    request.kind = 0x82;
    request.x = (s16)object->x;
    request.y = (s16)object->y;
    request.z = (s16)object->z;
    request.count = count;
    request.owner = object;
    spawned = func_8000590C(&request, 1);
    if (spawned != 0) {
        spawned->state = 0;
    }

    if (state->action->kind != 0) {
        if (state->resource != 0) {
            amSndStopXYZ(state->resource);
        }
        amSndPlayXYZ(state->action->kind, object->x, object->y,
                                     object->z, 4, &state->resource);
    }

    state->countdown--;
    if (state->countdown == 0) {
        state->state = 0xFF;
        state->action = 0;
    }
}
