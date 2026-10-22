#include "PR/ultratypes.h"

typedef struct Overlay53Entry {
    void *resource;
    s32 value4;
    s32 value8;
    s16 x;
    s16 y;
} Overlay53Entry;

extern s16 gOverlay53Offsets[];
extern s32 frontGetScreenMode(void);

/* Pinned DKR v77/v80 and JFG scans classify Overlay 53 as no donor. */
void overlay53CopyOffsetEntries(Overlay53Entry *source,
                                Overlay53Entry *destination, s32 selection,
                                s32 index) {
    s16 xOffset;
    s16 yOffset;
    s16 *offsets;

    index *= 4;
    if (frontGetScreenMode() == 1) {
        index += 8;
    }
    if (selection == 0) {
        offsets = &gOverlay53Offsets[index];
        xOffset = offsets[0];
        yOffset = offsets[1];
    } else {
        offsets = &gOverlay53Offsets[index];
        xOffset = offsets[2];
        yOffset = offsets[3];
    }

    while (source->resource != 0) {
        destination->resource = source->resource;
        destination->value4 = source->value4;
        destination->x = source->x + xOffset;
        destination->y = source->y + yOffset;
        destination->value8 = source->value8;
        source++;
        destination++;
    }
    destination->resource = 0;
}
