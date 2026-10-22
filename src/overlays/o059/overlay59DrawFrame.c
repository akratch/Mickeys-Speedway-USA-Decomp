#include "PR/ultratypes.h"

typedef struct Overlay59Entry {
    u8 pad00[9];
    s8 style;
    u8 pad0A[0x16];
    void *owner;
    u8 pad24[0x20];
} Overlay59Entry;

extern Overlay59Entry gOverlay59Entries[4];
extern s32 gOverlay59Styles[];
extern void fontUseFont(s32);
extern void fontBackground(s32, s32, s32, s32);
extern void fontColour(s32, s32, s32, s32, s32);
extern void fontPrintXY(s32, s32, s32, s32, s32);

/*
 * DKR v77/v80 and JFG contain generic white primitive-color and menu-frame
 * rendering, but no matching three-pass frame routine or exact object donor.
 */
void overlay59DrawFrame(s32 displayList, s32 index, s32 x, s32 y) {
    s32 style;
    void *owner;
    Overlay59Entry *entry;

    if (index >= 0 && index < 4) {
        entry = &gOverlay59Entries[index];
        x += 4;
        owner = entry->owner;
        if (owner != 0) {
            y += 12;
            style = gOverlay59Styles[entry->style];
            fontUseFont(0);
            fontBackground(0, 0, 0, 0);
            fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
            fontPrintXY(displayList, x - 1, y - 1, style, 0);
            fontColour(0, 0, 0, 0xFF, 0xFF);
            fontPrintXY(displayList, x + 1, y + 1, style, 0);
            fontColour(0xFF, 0, 0, 0xFF, 0xFF);
            fontPrintXY(displayList, x, y, style, 0);
        }
    }
}
