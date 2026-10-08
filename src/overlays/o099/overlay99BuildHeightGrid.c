#include "PR/ultratypes.h"

typedef struct Overlay99GridPoint {
    s16 reserved00;
    s16 reserved02;
    s16 height;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} Overlay99GridPoint;

typedef struct Overlay99Segment {
    u8 bytes[0x30];
} Overlay99Segment;

extern s32 gOverlay99CurrentGrid;
extern Overlay99GridPoint *gOverlay99Grids[];
extern s32 gOverlay99SegmentCount;
extern Overlay99Segment gOverlay99Segments[];
extern s32 gOverlay99GridWidth;
extern s32 gOverlay99GridHeight;
extern s32 gOverlay99WidthMinusOne;
extern s32 gOverlay99HeightMinusOne;
extern s32 gOverlay99Arg4;
extern s32 gOverlay99Arg5;
extern void overlay99ApplySegment(Overlay99Segment *segment, f32 scale);

/*
 * The negated magnitude is spelled as an expression rather than written back
 * through `value`: the write-back form reuses the value's own register, where
 * the target computes the negation into a separate caller-saved temporary and
 * leaves the clamped value dead.
 */
void overlay99BuildHeightGrid(f32 scale, void *unused, s32 widthMinusOne,
                              s32 heightMinusOne, s32 extra4, s32 extra5) {
    Overlay99GridPoint *point;
    s32 *widthPtr;
    s32 *heightPtr;
    s32 i;
    s32 value;

    point = gOverlay99Grids[gOverlay99CurrentGrid];
    if (point == 0) {
        return;
    }

    /* IDO homes an unreferenced named parameter to its incoming argument slot,
     * which is one instruction the target does not have; this empty test is
     * the only thing that stops the store. It is inert -- the second parameter
     * is dead in the target too -- and is recorded in docs/cleanup-queue.md.
     * Its position is load-bearing: before the null test or at the segment
     * loop the store comes back, and after the height grid is sized it also
     * exchanges the point/value pool colours. */
    if (unused != 0) {
    }
    widthPtr = &gOverlay99GridWidth;
    heightPtr = &gOverlay99GridHeight;
    *widthPtr = widthMinusOne + 1;
    *heightPtr = heightMinusOne + 1;
    gOverlay99WidthMinusOne = widthMinusOne;
    gOverlay99HeightMinusOne = heightMinusOne;
    gOverlay99Arg4 = extra4;
    gOverlay99Arg5 = extra5;

    i = *heightPtr * *widthPtr;
    while (i--) {
        point->height = 5;
        point++;
    }

    i = gOverlay99SegmentCount;
    while (i--) {
        overlay99ApplySegment(&gOverlay99Segments[i], scale);
    }

    widthPtr = &gOverlay99GridWidth;
    heightPtr = &gOverlay99GridHeight;
    i = *heightPtr * *widthPtr;
    point = gOverlay99Grids[gOverlay99CurrentGrid];
    while (i--) {
        value = point->height - 5;
        if (value < 0) {
            if (value < -40) {
                value = -40;
            }
            point->red = 0;
            point->green = 0;
            point->blue = 0;
            point->alpha = -value;
        } else if (value > 0) {
            if (value > 40) {
                value = 40;
            }
            point->red = 0xFF;
            point->green = 0xFF;
            point->blue = 0xFF;
            point->alpha = value;
        } else {
            point->alpha = 0;
        }
        point++;
    }
}
