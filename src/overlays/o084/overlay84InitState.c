#include "PR/ultratypes.h"

/* State bootstrap; pinned DKR objects provide no exact donor. */
typedef struct Overlay84InitState {
    s8 field0;
    s8 current;
    s8 status;
    u8 pad3[3];
    s8 mode;
    s8 timer;
    u8 pad8[0x34];
    s32 first;
    s32 second;
    u8 pad44[0x81];
    u8 active;
    u8 marked;
} Overlay84InitState;

typedef struct Overlay84InitObject {
    u8 pad0[0x64];
    Overlay84InitState *state;
} Overlay84InitObject;

extern Overlay84InitObject *gOverlay84Object;

void overlay84InitState(Overlay84InitObject *object, void *entry) {
    Overlay84InitState *state = object->state;

    state->field0 = 0;
    state->current = 0;
    state->status = -1;
    state->mode = 0;
    state->timer = 20;
    gOverlay84Object = object;
    state->first = 255;
    state->second = 255;
    state->active = 1;
    state->marked = 0;
}
