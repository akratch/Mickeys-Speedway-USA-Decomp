#include "PR/ultratypes.h"

typedef struct Overlay20Gfx {
    u32 w0;
    u32 w1;
} Overlay20Gfx;

typedef struct Overlay20Object {
    u8 pad0[0x39];
    u8 alpha;
    u8 pad3A[0x4A];
    void *resource;
} Overlay20Object;

extern void overlay20BuildTileCommands(Overlay20Gfx **gfxP, void *resource,
                                       s32 mode);

#define OVERLAY20_SHIFTL(value, shift, width) \
    (((u32)(value) & ((1U << (width)) - 1U)) << (shift))

#define OVERLAY20_SET_PRIM_COLOR(packet, alpha)                       \
    {                                                                  \
        Overlay20Gfx *macroCommand = (Overlay20Gfx *)(packet);          \
        macroCommand->w0 = 0xFA000000;                                  \
        macroCommand->w1 = OVERLAY20_SHIFTL(255, 24, 8) |               \
                           OVERLAY20_SHIFTL(255, 16, 8) |               \
                           OVERLAY20_SHIFTL(255, 8, 8) |                \
                           OVERLAY20_SHIFTL((alpha), 0, 8);             \
    }

/* DKR v77/v80 and JFG have generic GBI relatives, but no exact donor. */
void overlay20DrawResource(Overlay20Gfx **gfxP, Overlay20Object *object) {
    void *resource;
    s32 mode;

    resource = object->resource;
    if (resource != NULL) {
        mode = 10;
        if (object->alpha < 255) {
            OVERLAY20_SET_PRIM_COLOR((*gfxP)++, object->alpha);
            mode = 14;
        }
        overlay20BuildTileCommands(gfxP, resource, mode);
        OVERLAY20_SET_PRIM_COLOR((*gfxP)++, 255);
    }
}
