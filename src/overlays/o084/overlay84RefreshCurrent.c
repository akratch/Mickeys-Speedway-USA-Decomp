#include "PR/ultratypes.h"

typedef struct Overlay84Node {
    s16 angle;
    u8 pad2[0xA];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x14];
} Overlay84Node;

typedef struct Overlay84Resource {
    u8 pad0[0x17];
    u8 index;
    u8 pad18[8];
    Overlay84Node *nodes;
} Overlay84Resource;

typedef struct Overlay84State {
    u8 pad0[3];
    u8 value3;
    u8 pad4[5];
    u8 mode;
    u8 padA[0xA];
    s16 value14;
    s16 value16;
    u8 pad18[8];
    s32 angle20;
    u8 pad24[8];
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    u8 pad3C[0x88];
    u8 resource;
    u8 active;
    u8 marked;
} Overlay84State;

extern u32 gOverlay84InputFlags;
extern Overlay84Resource *overlay84GetResource(u8 resource);
extern void overlay84ReleaseResource(u8 resource);

/* DKR v77/v80 and JFG contain no exact donor for this resource refresh. */
void overlay84RefreshCurrent(void *unused, Overlay84State *state, s32 unused2) {
    struct {
        s32 changed;
        s32 reserved[2];
    } locals;
    Overlay84Resource *resource;
    Overlay84Node *node;

    locals.changed = 0;
    if ((gOverlay84InputFlags & 0xD000) != 0 && state->active != 0 &&
        state->marked == 0) {
        locals.changed = 1;
        resource = overlay84GetResource(state->resource);
        if (resource != 0) {
            node = &resource->nodes[resource->index - 1];
            state->x = node->x;
            state->y = node->y;
            state->z = node->z;
            state->angle20 = node->angle;
        }
    }
    resource = overlay84GetResource(state->resource);
    if (resource != 0 || locals.changed != 0) {
        state->scale = 1.0f;
        state->value14 = state->value16;
        state->mode = 2;
        state->value3 = 0;
        overlay84ReleaseResource(state->resource);
        state->resource = 0xFF;
    }
}
