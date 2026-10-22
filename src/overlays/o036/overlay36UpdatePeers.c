#include "PR/ultratypes.h"

#ifndef OVERLAY36_PEER_EFFECT_KIND
#define OVERLAY36_PEER_EFFECT_KIND 0xD7
#endif

typedef struct Overlay36Object Overlay36Object;
typedef struct Overlay36State Overlay36State;
typedef struct Overlay36SpawnedObject Overlay36SpawnedObject;

typedef struct Overlay36Action {
    u8 pad00[0x20];
    u16 kind;
} Overlay36Action;

struct Overlay36State {
    u8 pad000[0xA8];
    void *resource;
    u8 pad0AC[0x28];
    Overlay36SpawnedObject *peerResource;
    u8 pad0D8[0x92];
    s16 timer;
    u8 pad16C[0x2E];
    u8 state;
    u8 countdown;
    u8 pad19C[4];
    Overlay36Action *action;
    u8 pad1A4[0x1E1];
    u8 rank;
};

struct Overlay36Object {
    u8 pad00[0x0C];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x4C];
    Overlay36State *state;
};

typedef struct Overlay36SpawnRequest {
    s16 kind;
    s16 pad02;
    s16 x;
    s16 y;
    s16 z;
    s16 count;
    Overlay36Object *owner;
} Overlay36SpawnRequest;

struct Overlay36SpawnedObject {
    u8 pad00[0x3C];
    s32 state;
};

extern Overlay36Object *func_80005820(s32 index);
extern s32 overlay27Activate(void *resource);
extern Overlay36SpawnedObject *func_8000590C(
    Overlay36SpawnRequest *request, s32 mode);
extern void amSndStopXYZ(void *resource);
extern void amSndPlayXYZ(u16 kind, f32 x, f32 y, f32 z,
                                         s32 mode, void **resource);

void overlay36UpdatePeers(Overlay36Object *object) {
    Overlay36Object *peer;
    Overlay36SpawnRequest request;
    Overlay36State *peerState;
    Overlay36State *state;
    s32 index;

    state = object->state;
    index = 0;
    while ((peer = func_80005820(index++)) != 0) {
        if (peer != object) {
            peerState = peer->state;
            if (peerState->rank < state->rank && peerState->timer < 0x40 &&
                overlay27Activate(peerState->peerResource) == 0) {
                request.kind = OVERLAY36_PEER_EFFECT_KIND;
                request.x = (s16)peer->x;
                request.y = (s16)peer->y;
                request.z = (s16)peer->z;
                request.owner = peer;
                peerState->peerResource = func_8000590C(&request, 0);
                if (peerState->peerResource != 0) {
                    peerState->peerResource->state = 0;
                }
            }
        }
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
