#include "PR/ultratypes.h"

typedef struct Overlay41DrawItem {
    u16 unused0;
    u16 resourceIndex;
    u16 x;
    u16 y;
    u8 red;
    u8 green;
    u8 blue;
    u8 intensity;
} Overlay41DrawItem;

extern void **gOverlay41Resources;

extern void fontUseFont(s32 mode);
extern void fontBackground(s32 red, s32 green, s32 blue, s32 alpha);
extern s32 fontStringWidth(char *text, s32 font, s32 convertString);
extern void func_8004B0B8(s32 red, s32 green, s32 blue, s32 alpha,
                         s32 intensity);
extern void func_overlay_067_F0000000_18C6F40(void *displayList, s32 arg1,
                                               s32 x, s32 y, s32 width,
                                               s32 height, s32 intensity);
extern void fontPrintXY(void *displayList, s32 x, s32 y, void *resource,
                         s32 arg4);

void overlay41DrawItem(void *displayList,
                                       Overlay41DrawItem *item) {
    s32 x;
    s32 y;
    s32 width;

    fontUseFont(2);
    fontBackground(0, 0, 0, 0);
    width = fontStringWidth(gOverlay41Resources[item->resourceIndex], 2, 1) + 40;
    x = item->x - (width >> 1);
    y = item->y - 12;
    func_8004B0B8(0, 0, 0, 255, item->intensity);
    func_overlay_067_F0000000_18C6F40(displayList, 0, x, y, width, 24,
                                      item->intensity >> 1);
    fontPrintXY(displayList, item->x + 1, item->y + 1,
                  gOverlay41Resources[item->resourceIndex], 12);
    func_8004B0B8(item->red, item->green, item->blue, 255, item->intensity);
    fontPrintXY(displayList, item->x, item->y,
                  gOverlay41Resources[item->resourceIndex], 12);
}
