#include "PR/ultratypes.h"

/* Overlay 74 initializer; the exact DKR and JFG scans are negative. */
typedef struct Overlay74Transform {
    f32 scale;
} Overlay74Transform;

typedef struct Overlay74State {
    u8 strength;
    u8 channel;
    s8 minimum;
    s8 maximum;
} Overlay74State;

typedef struct Overlay74Object {
    u8 pad0[6];
    s16 flags;
    f32 scale;
    u8 padC[0x34];
    Overlay74Transform *transform;
    u8 pad44[0x20];
    Overlay74State *state;
} Overlay74Object;

typedef struct Overlay74Init {
    u8 pad0[0xA];
    s16 scale;
    u8 strength;
    u8 channel;
    s8 minimum;
    s8 maximum;
} Overlay74Init;

extern u32 D_800D3128;
extern f32 gOverlay74Scale;

u8 *func_80028F54(void);

void overlay74Init(Overlay74Object *object, Overlay74Init *init) {
    Overlay74State *state;
    u8 *status;

    status = func_80028F54();
    state = object->state;
    if ((*status != 0) ||
        ((((D_800D3128 << 5) >> 28) & (1 << init->channel)) != 0)) {
        object->flags |= 0x400;
    }

    state->strength = init->strength;
    state->minimum = init->minimum;
    state->maximum = init->maximum;
    state->channel = init->channel;
    object->scale = (f32) init->scale * gOverlay74Scale * object->transform->scale;
}
