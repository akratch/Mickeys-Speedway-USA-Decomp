#include "PR/ultratypes.h"

typedef struct Overlay84Meta {
    u8 pad00[0x10];
    u8 slot;
} Overlay84Meta;

typedef struct Overlay84Node {
    s16 angle;
    s16 tilt;
    u8 pad04[8];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x24];
    Overlay84Meta *meta;
    u8 pad40[4];
    s16 kind;
} Overlay84Node;

typedef struct Overlay84State {
    u8 initialized;
    s8 current;
    s8 last;
    u8 pad03;
    u8 phase;
    u8 timer;
    u8 pad06;
    s8 counter;
    u8 pad08;
    u8 mode;
    u8 pad0A[6];
    s16 tilt;
    s16 targetTilt;
    u8 pad14[2];
    s16 angle;
    u8 pad18[8];
    u32 nodeAngle;
    f32 height;
    f32 targetHeight;
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    u8 pad3C[8];
    Overlay84Node *nodes[32];
    u8 padC4;
    u8 active;
} Overlay84State;

typedef struct Overlay84Object {
    u8 pad00[0x0C];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x4C];
    Overlay84State *state;
} Overlay84Object;

extern Overlay84Node **func_8000572C(s32 *start, s32 *end);
extern s32 Arctanf(f32 x, f32 z);
extern void overlay84RefreshCurrent(Overlay84Object *, Overlay84State *, s32);
extern void func_overlay_084_F0000314_18D07F4(Overlay84Object *, Overlay84State *, s32);
extern void overlay84UpdateResource(Overlay84Object *, Overlay84State *, s32);

/* `start` and `end` are declared after `node` so their automatic homes land
 * at sp+0x44/sp+0x40: IDO 5.3 hands out homes descending from the top of the
 * frame in declaration order, and with them declared first they sat two words
 * too high. `scratch` is the function's eight-byte aggregate automatic, kept
 * at the end of the list so it stays below the temp region and holds the
 * frame at 0x58. */
void overlay84InitializeAndUpdate(Overlay84Object *object, s32 arg) {
    s32 i;
    Overlay84Node *initialNode;
    Overlay84Node **nodes;
    Overlay84Node *node;
    s32 start;
    s32 end;
    s16 tilt;
    Overlay84State *state;
    s32 scratch[2];

    (void)&scratch;

    state = object->state;
    if (state->initialized == 0) {
        nodes = func_8000572C(&start, &end);
        for (i = start; i < end; i++) {
            node = nodes[i];
            if (node->kind == 0x33) {
                s32 slot;

                slot = node->meta->slot;
                if (state->last < slot) {
                    state->last = slot;
                }
                state->nodes[slot] = node;
            }
        }

        initialNode = state->nodes[0];
        if (initialNode != 0) {
            state->angle = Arctanf(initialNode->x - object->x,
                                          initialNode->z - object->z);
            tilt = -initialNode->tilt;
            state->targetTilt = tilt;
            state->tilt = tilt;
            state->nodeAngle = initialNode->angle,
            state->height = state->targetHeight = initialNode->y;
            state->phase = 0;
            state->timer = 0;
            state->scale = 1.0f;
            state->x = initialNode->x;
            state->y = initialNode->y;
            state->z = initialNode->z;
        }
        state->initialized = 1;
        state->mode = 0;
    }

    switch (state->mode) {
    case 1:
        overlay84RefreshCurrent(object, state, arg);
        break;
    case 2:
        func_overlay_084_F0000314_18D07F4(object, state, arg);
        break;
    case 3:
        overlay84UpdateResource(object, state, arg);
        break;
    }
    state->active = 1;
}
