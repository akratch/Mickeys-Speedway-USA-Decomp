#include "PR/ultratypes.h"

typedef struct Overlay83Command { u32 w0; u32 w1; } Overlay83Command;
typedef struct Overlay83Strip {
    u8 pad00[0x20];
    u8 red;
    u8 green;
    u8 blue;
    u8 pad23[0xB];
    u8 count;
    u8 vertexIndex;
    u8 pad30[0x84];
} Overlay83Strip;

extern u8 D_80000000[];

#define SHIFTL(value, shift, width) \
    (((u32)(value) & ((1U << (width)) - 1U)) << (shift))
#define SET_PRIM(packet, red, green, blue, alpha) { \
    Overlay83Command *cmd = (Overlay83Command *)(packet); \
    cmd->w0 = 0xFA000000; \
    cmd->w1 = ((red) << 24) | ((green) << 16) | ((blue) << 8) | (alpha); \
}
#define SET_ENV(packet, red, green, blue, alpha) { \
    Overlay83Command *cmd = (Overlay83Command *)(packet); \
    cmd->w0 = 0xFB000000; \
    cmd->w1 = ((red) << 24) | ((green) << 16) | ((blue) << 8) | (alpha); \
}
#define VERTEX(packet, address, vertexCount, firstVertex) { \
    Overlay83Command *cmd = (Overlay83Command *)(packet); \
    cmd->w0 = SHIFTL(4, 24, 8) | \
              SHIFTL(((vertexCount) << 3) | ((u32)(address) & 6) | \
                         (firstVertex), 16, 8) | \
              SHIFTL(((vertexCount) * 10) + 8, 0, 16); \
    cmd->w1 = (u32)(address); \
}
#define POLYGON(packet, triangles, triangleCount, textured) { \
    Overlay83Command *cmd = (Overlay83Command *)(packet); \
    cmd->w0 = SHIFTL((((triangleCount) - 1) << 4) | (textured), 16, 8) | \
              SHIFTL(5, 24, 8) | SHIFTL((triangleCount) * 16, 0, 16); \
    cmd->w1 = (u32)(triangles); \
}

#ifdef NON_MATCHING
/* Workbench: structure-mismatch, 69 differing words, first mismatch +0x04.
 * Exact 77-instruction size and four-command CFG; packet arithmetic is reordered.
 * Trailing empty if (vertexCount) then if (count) are L100 occurrences; reverse
 * order is 73, and each probe alone is 72 and 71. */
void overlay83DrawStrip(Overlay83Command **displayList, Overlay83Strip *strip) {
    register Overlay83Command **savedDisplayList = displayList;
    s32 count;
    s32 doubledCount;
    s32 vertexCount;

    count = strip->count;
    if (count != 0) {
        SET_PRIM((*displayList)++, 255, 255, 255, 255);
        SET_ENV((*displayList)++, strip->red, strip->green, strip->blue, 255);
        doubledCount = count * 2;
        vertexCount = doubledCount + 2;
        VERTEX((*displayList)++,
               (u8 *)&strip[strip->vertexIndex] + 0x800000F0,
               vertexCount, 0);
        POLYGON((*savedDisplayList)++, D_80000000, doubledCount, 1);
        if (vertexCount) {
        }
        if (count) {
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o083/overlay83DrawStrip/func_overlay_083_F0000850_18D0010.s")
#endif

/* PLATEAU-HANDOFF:overlay83DrawStrip:start
 * symbol: overlay83DrawStrip
 * score: 69/77 words
 * frame: frameless
 * relocations: 2
 * first-mismatch: +0x4
 * summary: L100 empty-if pair 73 to 69 at delta 0. Leaf, zero p1 probes. Cursor force t0 is 68. Missing a0 copy at +0x4. D_80000000 identity fail-closed.
 * PLATEAU-HANDOFF:overlay83DrawStrip:end
 */
