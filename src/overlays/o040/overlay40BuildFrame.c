#include "PR/ultratypes.h"

typedef struct Overlay40FrameRecord {
    s16 left;
    s16 top;
    s16 right;
    s16 bottom;
    u32 color;
} Overlay40FrameRecord;

extern void frontDrawRectangles(void *displayList, s32 count,
                                Overlay40FrameRecord *records, s32 translucent);

/* `right`/`bottom`/`color` are declared before `records` so the eight
 * 12-byte records land at sp+0x40. `scratch` is the eight-byte aggregate
 * kept at the end of the list so the frame stays 0xB0. */
#line 22
#ifdef NON_MATCHING
void overlay40BuildFrame(void *displayList, s32 x, s32 y, s32 width,
                         s32 height, s32 red, s32 green, s32 blue, s32 alpha) {
#line 26
    s32 right;
    s32 bottom;
    s32 bottomPlus2;
    u32 color;
    Overlay40FrameRecord records[8];
    s32 scratch[2];
#line 40
    (void)&scratch;

#line 72
    right = x + width;
    bottom = y + height;
    bottomPlus2 = bottom + 2;
#line 78
    color = (red << 24) | (green << 16) | (blue << 8) | (alpha & 0xFF);

#line 95
    records[0].left = x - 2;
    records[0].top = y - 2;
    records[0].right = right + 2;
    records[0].bottom = y + 3;
    records[0].color = 0;

#line 101
    records[1].left = x;
    records[1].top = y;
    records[1].right = right + 1;
    records[1].bottom = y + 1;
    records[1].color = color;

#line 107
    records[2].left = x - 2;
    records[2].top = y - 2;
    records[2].right = x + 3;
    records[2].bottom = bottomPlus2;
    records[2].color = 0;

#line 113
    records[3].left = x;
    records[3].top = y + 1;
    records[3].right = x + 1;
    records[3].bottom = bottom;
    records[3].color = color;

#line 119
    records[4].left = right - 2;
    records[4].top = y - 2;
    records[4].right = right + 3;
#line 125
    records[4].bottom = *(volatile s32 *)&bottomPlus2;
#line 129
    records[4].color = 0;

#line 131
    records[5].left = right;
    records[5].top = y + 1;
    records[5].right = right + 1;
    records[5].bottom = bottom;
    records[5].color = color;

#line 137
    records[6].left = x - 2;
    records[6].top = bottom - 2;
    records[6].right = right + 2;
    records[6].bottom = bottom + 3;
    records[6].color = 0;

#line 143
    records[7].left = x;
    records[7].top = bottom;
    records[7].right = right + 1;
    records[7].bottom = bottom + 1;
    records[7].color = color;

#line 149
    frontDrawRectangles(displayList, 8, records, 0);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o040/overlay40BuildFrame/func_overlay_040_F00001A0_1886A50.s")
#endif

/* PLATEAU-HANDOFF:overlay40BuildFrame:start
 * symbol: overlay40BuildFrame
 * score: 75/81 words
 * frame: 0xB0
 * relocations: 1
 * first-mismatch: +0x4
 * summary: colour floor is 75; record homes now match at sp+0x40; store schedule remains
 * PLATEAU-HANDOFF:overlay40BuildFrame:end
 */
