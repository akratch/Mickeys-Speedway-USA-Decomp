#include "overlays/offset_records.h"

extern s16 gOverlay54Offsets[];
extern s32 frontGetScreenMode(void);

/* Shared Mickey family; no exact DKR v77/v80 or JFG donor was found. */
void overlay54CopyOffsetRecords(OverlayOffsetRecord *src, OverlayOffsetRecord *dst,
                                s32 mode, s32 index) {
    s32 xOffset;
    s32 yOffset;
    s16 *offsets;

    index *= 4;
    if (frontGetScreenMode() == 1) {
        index += 0x10;
    }

    offsets = &gOverlay54Offsets[index];
    if (mode < 2) {
        yOffset = offsets[1];
    } else {
        yOffset = offsets[3];
    }
    if (mode & 1) {
        xOffset = offsets[2];
    } else {
        xOffset = offsets[0];
    }

    while (src->link != 0) {
        dst->link = src->link;
        dst->value = src->value;
        src++;
        dst++;
        dst[-1].x = src[-1].x + xOffset;
        dst[-1].y = src[-1].y + yOffset;
        dst[-1].metadata = src[-1].metadata;
    }
    dst->link = 0;
}
