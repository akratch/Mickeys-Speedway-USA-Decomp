#include "PR/ultratypes.h"

typedef struct Overlay37Resource {
    u8 pad000[0x1A8];
    u16 flags1A8;
    u8 pad1AA[0x29E];
    f32 x448;
    f32 y44C;
    f32 z450;
} Overlay37Resource;

typedef struct Overlay37Object Overlay37Object;

typedef struct Overlay37State {
    u8 pad00[0xC];
    f32 phase;
    Overlay37Object *target;
} Overlay37State;

struct Overlay37Object {
    u8 pad00[4];
    s16 wave;
    u8 pad06[6];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x16];
    s16 angle;
    u8 pad30[0x34];
    Overlay37State *state;
};

extern f32 gOverlay37Period;
extern f32 func_8002A8C0(s32 angle);
extern void func_80006EA0(Overlay37Object *object);

/* Pinned DKR v77/v80 and JFG scans contain no exact source donor. */
void overlay37Update(Overlay37Object *object, s32 ticks) {
    Overlay37State *state;
    Overlay37Object *target;
    Overlay37Resource *resource;
    Overlay37Object *callObject;
    f32 period;
    f32 sine;
    s32 savedTicks;

    savedTicks = ticks;
    callObject = object;
    state = object->state;
    target = state->target;
    if (target != 0) {
        resource = (Overlay37Resource *)target->state;
        period = gOverlay37Period;
        object->x = ((Overlay37Resource *)target->state)->x448;
        object->y = resource->y44C;
        object->z = resource->z450;
        object->angle = target->angle;
        state->phase += ((f32)savedTicks / 60.0f) * 10.0f;
        if (period <= state->phase) {
            state->phase -= period;
        }
        sine = func_8002A8C0(
            (s32)((state->phase / period) * 65536.0f));
        callObject->wave = (s32)(4096.0f * sine);
        if (resource->flags1A8 & 1) {
            func_80006EA0(callObject);
        }
    }
}
