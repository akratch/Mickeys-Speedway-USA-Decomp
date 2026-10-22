#include "PR/ultratypes.h"

typedef struct Overlay84State {
    u8 pad0[9];
    u8 mode;
    u8 resource;
    u8 padB[0xBA];
    u8 active;
} Overlay84State;

typedef struct Overlay84Object {
    u8 pad0[0x64];
    Overlay84State *state;
} Overlay84Object;

typedef struct Overlay84Resource {
    u8 pad0[0x1C];
    f32 scale;
} Overlay84Resource;

extern Overlay84Object *gOverlay84Object;
extern Overlay84Resource *overlay84GetResource(u8 resource);
extern void func_overlay_041_F0000854_1887B8C(Overlay84Resource *resource, f32 scale,
                                       s32 arg2, s32 arg3);
extern void overlay84ReleaseResource(u8 resource);

/* DKR v77/v80 have no matching state/resource reset sequence. */
void overlay84ResetCurrent(void) {
    Overlay84State *state;
    Overlay84Resource *resource;

    if (gOverlay84Object != 0) {
        state = gOverlay84Object->state;
        state->mode = 3;
        state->active = 0;
        resource = overlay84GetResource(state->resource);
        if (resource != 0) {
            resource->scale = 1.0f;
            func_overlay_041_F0000854_1887B8C(resource, 2.0f, 2, 2);
            state->mode = 0;
            overlay84ReleaseResource(state->resource);
        }
    }
}
