#include "PR/ultratypes.h"

typedef struct Overlay31PaletteState {
    s32 unused;
    s32 count;
} Overlay31PaletteState;

typedef struct Overlay31PaletteCursor {
    u8 reserved00[8];
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
    Overlay31PaletteState *state;
} Overlay31PaletteCursor;

typedef struct Overlay31Item {
    u32 flags;
    u8 reserved04[0x1B];
    u8 paletteCount;
    u32 *palette;
    u16 paletteId;
} Overlay31Item;

extern s32 gOverlay31ItemCount;
extern Overlay31Item **gOverlay31Items;

extern void initColourCycle(Overlay31PaletteCursor *cursor,
                                      u16 paletteId);
extern void updateColourCycle(Overlay31PaletteCursor *cursor,
                                         void *step);
extern void *mmAlloc(s32 size, s32 tag);

void overlay31BuildPalettes(void) {
    Overlay31Item *item;
    s32 count;
    s32 i;
    s32 j;
    Overlay31PaletteCursor cursor;

    for (i = 0; i < gOverlay31ItemCount; i++) {
        item = gOverlay31Items[i];
        if (item->flags & 0x400) {
            for (j = 0; j < i; j++) {
                if (item->paletteId == gOverlay31Items[j]->paletteId) {
                    item->palette = gOverlay31Items[j]->palette;
                    item->paletteCount = cursor.state->count;
                    if (item->paletteCount == 0) {
                        item->paletteCount = 1;
                    }
                    j = i;
                }
            }

            if (item->palette == 0) {
                u32 *palette;

                initColourCycle(&cursor, item->paletteId);
                count = cursor.state->count;
                if (count == 0) {
                    count = 1;
                }
                if (count >= 0x100) {
                    count = 0xFF;
                }

                item->palette = mmAlloc(count * 4, 0x8C);
                item->paletteCount = count;
                palette = item->palette;
                for (j = 0; j < count; j++) {
                    *palette++ = ((u32)cursor.red << 24) |
                                 ((u32)cursor.green << 16) |
                                 ((u32)cursor.blue << 8) | cursor.alpha;
                    updateColourCycle(&cursor, (void *)1);
                }
            }
        }
    }
}
