#ifndef OVERLAY_067_H
#define OVERLAY_067_H

#include "PR/ultratypes.h"

typedef struct Overlay67Point {
    s8 useXOffset0;
    s8 x0;
    s8 useYOffset0;
    s8 y0;
    s8 useXOffset1;
    s8 x1;
    s8 useYOffset1;
    s8 y1;
    s8 alpha;
} Overlay67Point;

typedef struct Overlay67Vertex {
    s16 x0;
    s16 y0;
    s16 x1;
    s16 y1;
    u32 color;
} Overlay67Vertex;

extern Overlay67Point *gOverlay67PointSets[];

void frontDrawRectangles(void *displayList, s32 count,
                        Overlay67Vertex *vertices, s32 flags);
void overlay67BuildVertices(void *displayList, u8 setIndex, s32 x, s32 y,
                            s32 xOffset, s32 yOffset, u32 color);

#endif
