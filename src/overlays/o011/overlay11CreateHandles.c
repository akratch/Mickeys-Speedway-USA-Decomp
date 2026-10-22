#include "PR/ultratypes.h"

typedef struct Overlay11Resources {
    u8 pad0[0x58];
    void *resource58;
    void *resource5C;
    u8 pad60[0x270];
    void *resource2D0;
} Overlay11Resources;

extern Overlay11Resources *D_8007C0B8;
extern u8 gOverlay11DirectResource[];
extern void *gOverlay11Handles[4];
extern s32 gOverlay11Mode;
extern void *overlay45CreateDescriptor(void *resource, s32 x, s32 y, s32 mode);
extern void overlay11SetValue(void *handle, s32 value);

/* DKR v77/v80 contain only generic fixed-resource creation sequences. */
void overlay11CreateHandles(void) {
    gOverlay11Handles[0] = overlay45CreateDescriptor(D_8007C0B8->resource2D0,
                                                160, 40, 4);
    gOverlay11Handles[1] = overlay45CreateDescriptor(gOverlay11DirectResource,
                                                160, 120, 4);
    gOverlay11Handles[2] = overlay45CreateDescriptor(D_8007C0B8->resource58,
                                                120, 120, 4);
    gOverlay11Handles[3] = overlay45CreateDescriptor(D_8007C0B8->resource5C,
                                                200, 120, 4);
    overlay11SetValue(gOverlay11Handles[0], 0);
    overlay11SetValue(gOverlay11Handles[1], 0);
    overlay11SetValue(gOverlay11Handles[2], 0);
    overlay11SetValue(gOverlay11Handles[3], 0);
    gOverlay11Mode = 3;
}
