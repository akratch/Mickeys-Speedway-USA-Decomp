#include "PR/ultratypes.h"

typedef struct Overlay14Entry {
    s16 count;
    s16 selected;
    u8 *text;
    s16 kind;
    u8 padA[6];
} Overlay14Entry;

extern s32 gOverlay14ValueC0;
extern s32 gOverlay14DataBase;
extern s32 gOverlay14Args34;
extern Overlay14Entry *gOverlay14Entries;
extern void overlay14BuildPanel(s32, void *, s32, s32, s32, s32, s32);
extern s32 overlay14Dispatch();
extern void overlay14DrawPrimitive();
extern s32 overlay14ValidateEntry(s16);

/* Matched 2026-09-16 (lane lm-a), 188/188 words at frame 0x80, unforced.
 * The last twelve words were a v0/v1 exchange between the entries-base web
 * and the index reload. The base web had v0 forbidden with no v0-coloured web
 * in its blocks: the only v0 values there were the unused results of the two
 * dispatch calls before the loop head (a call's result is a v0 web that
 * occupies the call's block; docs/lastmile-region-boundary.md). The calls
 * whose results this body never reads are declared through the void
 * `overlay14DrawPrimitive` surface the sibling TU already uses for the same
 * shapes, and the two value-returning calls keep `overlay14Dispatch`; the
 * ROM cannot distinguish them, every one is the same runtime-table call.
 * The outer while and the generated entry stride (no declared offset carrier)
 * are the previous lanes' work and stay load-bearing. */
void func_overlay_014_F0001540_1870E18(s32 context) {
    u8 saved;
    s32 index;
    s32 cellWidth;
    s32 remaining;
    s32 x;
    s32 y;
    s32 opacity;
    u8 *cursor;
    void *drawArg;
    s32 first;

    overlay14BuildPanel(context, &gOverlay14DataBase, 0x5C, 0x14, 0xD0, 0x58,
                        (gOverlay14ValueC0 * 0xA0) >> 8);
    index = 0;
    cellWidth = overlay14Dispatch(2);
    remaining = (0x58 / cellWidth) - 1;
    overlay14DrawPrimitive(2);
    overlay14DrawPrimitive(0, 0, 0, 0);
    y = ((0x58 - (remaining * cellWidth)) >> 1) + 0x14;
    opacity = (gOverlay14ValueC0 * 0xFF) >> 8;
    while ((index < gOverlay14Entries->count) && (remaining > 0)) {


        first = 1; x = 0x60;
        if (index == gOverlay14Entries->selected)
            overlay14DrawPrimitive(0xFF, 0xFF, 0xFF, 0xFF, opacity);
        else
            overlay14DrawPrimitive(0, 0xC0, 0xC0, 0xFF, opacity);
        cursor = gOverlay14Entries[index].text;
        if (overlay14ValidateEntry(
                gOverlay14Entries[index].kind) == 0) cursor = 0;
        if ((cursor != 0) && (remaining > 0)) {
            do {
                cursor = (u8 *)overlay14Dispatch(2, cursor, 0xC8, &drawArg, 0);
                if (cursor != 0) {
                    saved = *cursor; *cursor = 0;
                    overlay14DrawPrimitive(context, x, y, drawArg, 0);
                    *cursor = saved; y += cellWidth;
                    if (first != 0) { x += 8; first = 0; }
                    remaining--;
                }
            } while ((cursor != 0) && (remaining > 0));
        }
        index++;


    }
    overlay14DrawPrimitive(0, 0, 0, 0);
    overlay14DrawPrimitive(0xFF, 0xC0, 0, 0xFF, opacity);
    if ((index < gOverlay14Entries->count) || (cursor != 0))
        overlay14DrawPrimitive(context, 0xC4, 0x6C, &gOverlay14Args34, 0xC);
}

