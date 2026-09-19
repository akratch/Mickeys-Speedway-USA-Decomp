#include "PR/ultratypes.h"

typedef struct Overlay73Command {
    u32 w0;
    u32 w1;
} Overlay73Command;

typedef struct Overlay73DrawState {
    u8 pad00[0x78];
    void *resource;
    u8 vertexBank;
} Overlay73DrawState;

typedef struct Overlay73DrawObject {
    u8 pad00[0x39];
    u8 alpha;
    u8 pad3A[0x2A];
    Overlay73DrawState *state;
} Overlay73DrawObject;

extern u8 D_80000000[];
extern void func_8002409C(Overlay73Command **commands, s32 context,
                         Overlay73DrawObject *object, f32 scale, f32 extra);
extern void func_80034554(Overlay73Command **commands, void *resource,
                         s32 mode, s32 flags);
extern void func_800241BC(Overlay73Command **commands);

/* Workbench: exact 78-word / 0x30 frame, 42 masked, first +0x1C.
 * Writing the 0x05710080 word before D_80000000 closes that pair's birth order
 * (structural 4 to 2). Remaining: vertices spill +0x28 vs +0x2C, and the
 * vertexBank load in v0 vs a ring temp. Colour landscape floor 39 (w43 to a1). */
#ifdef NON_MATCHING
void func_overlay_073_F0000D70_18CB830(Overlay73Command **commands,
                                       s32 context,
                                       Overlay73DrawObject *object) {
    Overlay73Command *command;
    u8 * volatile vertices;
    Overlay73DrawState *state;
    u32 physicalVertices;
    s32 vertexBank;
    s32 vertexOffset;

    state = object->state;
    if (state->resource != NULL) {
        vertexBank = state->vertexBank;
        vertexOffset = (vertexBank << 2) - vertexBank;
        vertexOffset <<= 1;
        vertexOffset = (vertexOffset << 2) + vertexOffset;
        vertexOffset <<= 1;
        vertices = (u8 *)state + vertexOffset;
        func_8002409C(commands, context, object, 1.0f, 0.0f);

        command = *commands;
        *commands = command + 1;
        command->w0 = 0xFA000000; command->w1 = object->alpha | 0xFFFFFF00;

        func_80034554(commands, state->resource, 0xE, 0);

        command = *commands;
        *commands = command + 1;
        physicalVertices = (u32)vertices + 0x80000000;
        command->w0 = (((((physicalVertices) & 6) | 0x30) & 0xFF)
                       << 16) |
                      0x04000000 | 0x44;
        command->w1 = physicalVertices;

        command = *commands;
        *commands = command + 1;
        command->w0 = 0x05710080; command->w1 = (u32)D_80000000;

        command = *commands;
        *commands = command + 1;
        command->w0 = 0xFA000000; command->w1 = object->alpha | 0xFFFFFF00;
        func_800241BC(commands);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o073/overlay73Draw/func_overlay_073_F0000D70_18CB830.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_073_F0000D70_18CB830:start
 * symbol: func_overlay_073_F0000D70_18CB830
 * score: 42/78 words
 * frame: 0x30
 * relocations: 5
 * first-mismatch: +0x1C
 * summary: w0-first closed 0x0571/D_80000000 birth order (structural 4 to 2). Residual: spill +0x28 vs +0x2C and LBU v0 vs t7. Colour floor 39. L145/L97/lineno exhausted.
 * PLATEAU-HANDOFF:func_overlay_073_F0000D70_18CB830:end
 */
