#include "PR/ultratypes.h"

extern s32 gOverlay61ItemCountReloc;
extern s32 gOverlay61SelectedIndexReloc;
extern s32 gOverlay61TopIndexReloc;
extern u8 gOverlay61ItemsReloc[];
extern u8 gOverlay61UpLabelReloc[];
extern u8 gOverlay61DownLabelReloc[];

extern void fontUseFont(s32 mode, s32 *index);
extern void fontBackground(s32, s32, s32, s32);
extern void fontColour(s32, s32, s32, s32, s32);
extern void fontPrintXY(s32, s32, s32, void *, s32);
extern void func_overlay_061_F00003C0_18BF788(
    s32 context, s32 y, void *item, s32 selected);

void overlay61DrawList(s32 context) {
    s32 index;
    s32 y;
    s32 remaining;

    if (gOverlay61SelectedIndexReloc < gOverlay61TopIndexReloc) {
        gOverlay61TopIndexReloc = gOverlay61SelectedIndexReloc;
    } else if (gOverlay61SelectedIndexReloc >=
               gOverlay61TopIndexReloc + 7) {
        gOverlay61TopIndexReloc = gOverlay61SelectedIndexReloc - 6;
    }

    fontUseFont(2, &gOverlay61TopIndexReloc);
    fontBackground(0, 0, 0, 0);

    index = gOverlay61TopIndexReloc;
    y = 0x32;
    /* IDO schedules both relocated bases before the counter on this line. */
    remaining = 6; do { if (index >= gOverlay61ItemCountReloc) {
        func_overlay_061_F00003C0_18BF788(context, y, 0, 0);
        } else {
            func_overlay_061_F00003C0_18BF788(
                context, y, &gOverlay61ItemsReloc[index << 6],
                index++ == gOverlay61SelectedIndexReloc);
        }
        y += 0x18;
    } while (remaining--);

    fontColour(0xFF, 0xFF, 0, 0xFF, 0xFF);
    if (gOverlay61TopIndexReloc != 0) {
        fontPrintXY(
            context, 0xA0, 0x28, gOverlay61UpLabelReloc, 4);
    }
    if (gOverlay61TopIndexReloc + 7 < gOverlay61ItemCountReloc) {
        fontPrintXY(
            context, 0xA0, 0xD8, gOverlay61DownLabelReloc, 4);
    }
}
