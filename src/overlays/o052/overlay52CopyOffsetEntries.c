#include "PR/ultratypes.h"

typedef struct Overlay52Entry {
    void *resource;
    s32 value4;
    s32 value8;
    s16 x;
    s16 y;
} Overlay52Entry;

extern s16 gOverlay52Offsets[];
extern s32 frontGet2PlayerSplit(Overlay52Entry *source,
                                          Overlay52Entry *destination);
extern s32 frontGetScreenMode(Overlay52Entry *source,
                                            Overlay52Entry *destination);

void overlay52CopyOffsetEntries(Overlay52Entry *source,
                                Overlay52Entry *destination, s32 selection,
                                s32 index) {
    s16 xOffset;
    s16 yOffset;

    index *= 4;
    if (frontGet2PlayerSplit(source, destination) != 0) {
        index += 0x14;
    } else if (frontGetScreenMode(source, destination) & 1) {
        index += 0x28;
    }

    if (selection == 0) {
        xOffset = gOverlay52Offsets[index];
        yOffset = gOverlay52Offsets[index + 1];
    } else {
        xOffset = gOverlay52Offsets[index + 2];
        yOffset = gOverlay52Offsets[index + 3];
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
