#include "PR/ultratypes.h"

typedef struct Overlay84State {
    u8 pad0[9];
    u8 mode;
    u8 resource;
    u8 padB[0xBA];
    u8 active;
    u8 marked;
} Overlay84State;

typedef struct Overlay84Resource {
    u8 pad0[0x1C];
    f32 scale;
} Overlay84Resource;

extern u32 D_800D31B8;
extern Overlay84Resource *overlay84GetResource(u8 resource);
extern void func_overlay_041_F0000854_1887B8C(Overlay84Resource *resource, f32 scale,
                                       s32 arg2, s32 arg3);
extern void overlay84ReleaseResource(u8 resource);

/* DKR v77/v80 have no matching input-gated resource update. */
void overlay84UpdateResource(void *unused, Overlay84State *state, s32 value) {
    Overlay84Resource *resource;

    if ((D_800D31B8 & 0xD000) != 0 && state->active != 0 &&
        state->marked == 0) {
        resource = overlay84GetResource(state->resource);
        if (resource != 0) {
            resource->scale = 1.0f;
            func_overlay_041_F0000854_1887B8C(resource, (f32)value, value, value);
        }
    }
    resource = overlay84GetResource(state->resource);
    if (resource != 0) {
        state->mode = 0;
        overlay84ReleaseResource(state->resource);
    }
}
