#include "PR/ultratypes.h"

typedef struct Overlay96Command {
    u32 w0;
    u32 w1;
} Overlay96Command;

typedef struct Overlay96Object {
    u8 pad00[0x64];
    void *vertices;
} Overlay96Object;

typedef struct Overlay96DrawLocals {
    void *vertices;
} Overlay96DrawLocals;

extern void texDPTextureX(Overlay96Command **commands, void *resource,
                          s32 mode, s32 flags);
extern u8 D_80000000[];

#define OVERLAY96_SHIFTL(value, shift, width) \
    (((u32)(value) & ((1U << (width)) - 1U)) << (shift))

#define OVERLAY96_PIPE_SYNC(packet)                                      \
    {                                                                    \
        Overlay96Command *macroCommand = (Overlay96Command *)(packet);    \
        macroCommand->w0 = 0xE7000000;                                   \
        macroCommand->w1 = 0;                                            \
    }

#define OVERLAY96_PRIM_WHITE(packet)                                     \
    {                                                                    \
        Overlay96Command *macroCommand = (Overlay96Command *)(packet);    \
        macroCommand->w0 = 0xFA000000;                                   \
        macroCommand->w1 = 0xFFFFFFFF;                                   \
    }

#define OVERLAY96_ENV_WHITE(packet)                                      \
    {                                                                    \
        Overlay96Command *macroCommand = (Overlay96Command *)(packet);    \
        macroCommand->w0 = 0xFB000000;                                   \
        macroCommand->w1 = 0xFFFFFF00;                                   \
    }

#define OVERLAY96_LOAD_VERTICES(packet, address)                         \
    {                                                                    \
        Overlay96Command *macroCommand = (Overlay96Command *)(packet);    \
        void *vertexAddress = (void *)(address);                          \
        macroCommand->w0 =                                                \
            ((((((u32)vertexAddress & 6U) | 0x40U) & 0xFFU) << 16) |     \
             0x04000000U) |                                              \
            0x58U;                                                       \
        macroCommand->w1 = (u32)vertexAddress;                            \
    }

#define OVERLAY96_DRAW_TRIANGLES(packet, address, count, textured)       \
    {                                                                    \
        Overlay96Command *macroCommand = (Overlay96Command *)(packet);    \
        macroCommand->w0 =                                                \
            OVERLAY96_SHIFTL((((count) - 1) << 4) | (textured), 16, 8) | \
            OVERLAY96_SHIFTL(5, 24, 8) |                                 \
            OVERLAY96_SHIFTL((count) * 16, 0, 16);                        \
        macroCommand->w1 = (u32)(address);                                \
    }

/* DKR v77/v80 and JFG have generic display-list relatives, but no donor. */
void overlay96DrawObject(Overlay96Command **commands, s32 unused1,
                         s32 unused2, Overlay96Object *object) {
    Overlay96DrawLocals locals;

    locals.vertices = object->vertices;
    OVERLAY96_PIPE_SYNC((*commands)++);
    OVERLAY96_PRIM_WHITE((*commands)++);
    OVERLAY96_ENV_WHITE((*commands)++);
    texDPTextureX(commands, NULL, 0x16, 0);
    OVERLAY96_LOAD_VERTICES(
        (*commands)++, (void *)((u32)locals.vertices + 0x80000000U));
    OVERLAY96_DRAW_TRIANGLES((*commands)++, D_80000000, 12, 0);
    OVERLAY96_PIPE_SYNC((*commands)++);
}
