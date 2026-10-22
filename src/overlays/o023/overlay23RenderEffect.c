#include "PR/ultratypes.h"

typedef struct Gfx {
    u32 w0;
    u32 w1;
} Gfx;

#define OVERLAY23_PIPE_SYNC(packet)                \
    {                                              \
        Gfx *macroCommand = (Gfx *)(packet);       \
        macroCommand->w0 = 0xE7000000;             \
        macroCommand->w1 = 0;                      \
    }

#define OVERLAY23_ENV_WHITE(packet)                \
    {                                              \
        Gfx *macroCommand = (Gfx *)(packet);       \
        macroCommand->w0 = 0xFB000000;             \
        macroCommand->w1 = 0xFFFFFF00;             \
    }

typedef struct Overlay23RenderState {
    u8 pad00[8];
    f32 scale;
    u8 pad0C[4];
    s16 value0;
    s16 value1;
    u8 pad14[0x0C];
    s32 resource;
} Overlay23RenderState;

typedef struct Overlay23RenderObject {
    u8 pad00[0x0C];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x21];
    u8 alpha;
    u8 pad3A[0x16];
    void *renderResource;
    u8 pad54[0x10];
    Overlay23RenderState *state;
} Overlay23RenderObject;

typedef struct Overlay23RenderPacket {
    s16 value0;
    s16 value1;
    u8 pad04[2];
    s16 mode;
    f32 scale;
    f32 unitScale;
    f32 x;
    f32 y;
    f32 z;
    s32 color;
    s32 resource;
} Overlay23RenderPacket;

/* The overlay relocation stream resolves these two call sites independently,
 * although the relocatable object stores the same zero-valued proxy symbol. */
extern void overlay23CallReloc();

/* Block-local command temporaries reproduce the original display-list macro
 * expansion and its live ranges. */
void overlay23RenderEffect(Overlay23RenderObject *object, Gfx **displayList,
                           void *renderArg0, void *renderArg1) {
    Overlay23RenderState *state;
    Overlay23RenderPacket packet;

    state = object->state;
    overlay23CallReloc();

    packet.mode = 3;
    packet.color = 0x3333;
    packet.value0 = state->value0;
    packet.value1 = state->value1;
    packet.scale = state->scale;
    packet.unitScale = 1.0f;
    packet.x = object->x;
    packet.y = object->y;
    packet.z = object->z;
    packet.resource = state->resource;

    OVERLAY23_PIPE_SYNC((*displayList)++);
    OVERLAY23_ENV_WHITE((*displayList)++);

    overlay23CallReloc(displayList, renderArg0, renderArg1, object,
                       object->renderResource, &packet, 14, object->alpha);
}
