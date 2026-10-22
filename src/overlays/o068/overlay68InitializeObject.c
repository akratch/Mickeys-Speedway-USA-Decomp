#include "PR/ultratypes.h"

/*
 * DKR v77/v80 and JFG source searches found only generic colour/opacity and
 * fixed-point initialization idioms, not an exact donor for this state shape.
 */

typedef struct Overlay68ObjectState {
    void *resource;
    f32 value;
    s16 angle;
    s16 timer;
    s16 phase;
    u8 active;
    u8 opacity;
} Overlay68ObjectState;

typedef struct Overlay68Object {
    s16 red;
    s16 green;
    s16 blue;
    u8 pad6[0x5E];
    Overlay68ObjectState *state;
} Overlay68Object;

typedef struct Overlay68ObjectInit {
    u8 pad0[0xA];
    u8 red;
    u8 green;
    u8 blue;
    u8 padD[3];
    void *resource;
} Overlay68ObjectInit;

extern void func_8005AD64(Overlay68Object *object, s32 arg1, s32 arg2, f32 arg3);

void overlay68InitializeObject(Overlay68Object *object, Overlay68ObjectInit *init) {
    Overlay68ObjectState *state;

    state = object->state;
    object->red = init->red << 8;
    object->green = init->green << 8;
    object->blue = init->blue << 8;
    state->resource = init->resource;
    state->timer = 0;
    state->phase = 0;
    state->angle = 0;
    state->active = 1;
    state->opacity = 0x80;
    state->value = 0.0f;
    func_8005AD64(object, 0, -1, 0.0f);
}
