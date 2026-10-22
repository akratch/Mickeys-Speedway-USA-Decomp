#include "PR/ultratypes.h"

typedef struct Overlay13Resource {
    u8 pad00[6];
    u16 width;
    u16 height;
} Overlay13Resource;

extern void *gOverlay13Resource0;
extern void *gOverlay13Resource1;
extern Overlay13Resource *gOverlay13Resource2;
extern s32 gOverlay13Enabled;
extern u8 gOverlay13Records[];
extern u8 gOverlay13Output[];
extern s32 gOverlay13Active;

extern void *texLoadSprite(s32 assetId, s32 arg1);
extern void *texLoadTexture(s32 resourceId);

/* Pinned DKR v77/v80 have no O13 donor; JFG's ambiguous hit is at +0xB0C. */
void overlay13Initialize(void) {
    u8 *record;
    u8 *entry;
    Overlay13Resource *resource;
    s32 recordIndex;
    s32 entryIndex;
    s32 width;
    s32 height;

    gOverlay13Resource0 = texLoadSprite(0x2C, 0);
    gOverlay13Resource1 = texLoadSprite(0x2D, 0);
    gOverlay13Resource2 = texLoadTexture(0x6A);

    record = gOverlay13Records;
    recordIndex = 0;
    do {
        record[6] = 0;
        entryIndex = 0;
        entry = record;
        do {
            entryIndex++;
            entry += 0x28;
            entry[0x18] = 0xFF;
            entry[0x19] = 0xFF;
            entry[0x1A] = 0xFF;
            entry[0x1B] = 0xFF;
            entry[0x22] = 0xFF;
            entry[0x23] = 0xFF;
            entry[0x24] = 0xFF;
            entry[0x25] = 0xFF;
            entry[0x2C] = 0xFF;
            entry[0x2D] = 0xFF;
            entry[0x2E] = 0xFF;
            entry[0x2F] = 0xFF;
            entry[0x0E] = 0xFF;
            entry[0x0F] = 0xFF;
            entry[0x10] = 0xFF;
            entry[0x11] = 0xFF;
        } while (entryIndex != 2);
        recordIndex++;
        record += 0x80;
    } while (recordIndex != 0x20);

    gOverlay13Enabled = 0;
    resource = gOverlay13Resource2;
    if (resource != 0) {
        width = (resource->width - 1) << 5;
        height = (resource->height - 1) << 5;
        *(s16 *)(gOverlay13Output + 0x08) = width;
        *(s16 *)(gOverlay13Output + 0x0C) = width;
        *(s16 *)(gOverlay13Output + 0x0E) = height;
        *(s16 *)(gOverlay13Output + 0x18) = width;
        *(s16 *)(gOverlay13Output + 0x1A) = height;
        *(s16 *)(gOverlay13Output + 0x1E) = height;
    }
    gOverlay13Active = 1;
}
