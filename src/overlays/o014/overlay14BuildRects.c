#include "PR/ultratypes.h"

typedef struct Overlay14PackedRect {
    s8 useAlternateX0;
    s8 x0;
    s8 useAlternateY0;
    s8 y0;
    s8 useAlternateX1;
    s8 x1;
    s8 useAlternateY1;
    s8 y1;
    s8 intensity;
} Overlay14PackedRect;

typedef struct Overlay14RenderRect {
    s16 x0;
    s16 y0;
    s16 x1;
    s16 y1;
    s32 intensity;
} Overlay14RenderRect;

extern void frontDrawRectangles(void *context, s32 count,
                                      Overlay14RenderRect *rects, s32 flags);

/* Pinned DKR v77/v80 and JFG object scans found no exact donor. */
void overlay14BuildRects(void *context, const Overlay14PackedRect *packed,
                         s32 baseX, s32 baseY, s32 alternateX,
                         s32 alternateY, s32 intensityScale) {
    Overlay14RenderRect *rect;
    Overlay14RenderRect rects[24];
    s32 count;

    rect = rects;
    count = 0;
    while ((packed->useAlternateX0 >= 0) && (count != 24)) {
        rect->x0 = packed->x0 + baseX;
        if (packed->useAlternateX0 != 0) {
            rect->x0 += alternateX;
        }

        rect->y0 = packed->y0 + baseY;
        if (packed->useAlternateY0 != 0) {
            rect->y0 += alternateY;
        }

        rect->x1 = packed->x1 + baseX;
        if (packed->useAlternateX1 != 0) {
            rect->x1 += alternateX;
        }

        rect->y1 = packed->y1 + baseY;
        if (packed->useAlternateY1 != 0) {
            rect->y1 += alternateY;
        }

        rect->intensity = (packed->intensity * intensityScale) >> 7;
        count++;
        packed++;
        rect++;
    }

    frontDrawRectangles(context, count, rects, 0);
}
