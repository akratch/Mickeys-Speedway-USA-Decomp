#include "PR/ultratypes.h"

typedef struct Overlay84Node {
    s16 pad0;
    s16 tilt;
    u8 pad4[8];
    f32 x;
    f32 y;
    f32 z;
} Overlay84Node;

typedef struct Overlay84State {
    u8 pad0;
    s8 current;
    s8 status;
    u8 pad3[0xD];
    s16 tilt;
    u8 pad12[4];
    s16 angle;
    u32 flags;
    u8 pad1C[8];
    f32 height;
    u8 pad28[0x1C];
    Overlay84Node *nodes[32];
} Overlay84State;

typedef struct Overlay84Object {
    u8 pad0[0xC];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x4C];
    Overlay84State *state;
} Overlay84Object;

extern Overlay84Object *gOverlay84Object;
extern s32 Arctanf(f32 x, f32 z);

/*
 * DKR v77/v80 contain many generic arctan2 object-heading calculations, but
 * neither DKR nor JFG has this indexed-node selection/update sequence or an
 * exact object donor.
 */
void overlay84SelectCurrent(s32 index) {
    register Overlay84State *state;
    register Overlay84Object *object;
    volatile u16 scratch[2];

    (void)&scratch;
    object = gOverlay84Object;
    if (object != 0) {
        Overlay84Node *node;
        state = object->state;
        if (state->status != -1) {
            state->current = index;
            object = gOverlay84Object;
            node = state->nodes[state->current];
            state->angle =
                Arctanf(node->x - object->x, node->z - object->z);
            state->tilt = -node->tilt;
            state->height = node->y;
            state->flags &= ~1;
        }
    }
}
