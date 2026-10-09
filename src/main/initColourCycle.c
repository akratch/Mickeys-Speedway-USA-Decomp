/*
 * PROVENANCE: body adapted from Jet Force Gemini's public decompilation,
 * src/textures.c:initColourCycle. Mickey's symbols, ABI, linked bytes, and
 * relocations remain authoritative.
 */

#include "PR/ultratypes.h"

typedef struct ColourCycle {
    s32 unk0;
    s32 unk4;
    u8 unk8;
    u8 unk9;
    u8 unkA;
    u8 unkB;
    struct ColourCycle *unkC;
} ColourCycle;

extern void *func_800056A4(s32 tableIndex);
extern void resetColourCycle(ColourCycle *cycle);

void initColourCycle(ColourCycle *cycle, s32 tableIndex) {
    cycle->unkC = (ColourCycle *)func_800056A4(tableIndex);
    resetColourCycle(cycle);
}
