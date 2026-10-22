#include "overlays/overlay_067.h"

/* Overlay 67, ADR 0006 module TU; no exact DKR/JFG object match was found. */
void overlay67BuildVertices(void *displayList, u8 setIndex, s32 x, s32 y,
                            s32 xOffset, s32 yOffset, u32 color) {
    register Overlay67Point *point;
    register Overlay67Vertex *vertex;
    Overlay67Vertex vertices[24];
    register s32 count;
    register s32 alpha;

    point = gOverlay67PointSets[setIndex];
    alpha = color & 0xFF;
    vertex = vertices;
    count = 0;
    if ((color &= ~0xFF, point->useXOffset0 >= 0)) {
        do {
            vertex->x0 = point->x0 + x;
            if (point->useXOffset0 != 0) {
                vertex->x0 += xOffset;
            }
            vertex->y0 = point->y0 + y;
            if (point->useYOffset0 != 0) {
                vertex->y0 += yOffset;
            }
            vertex->x1 = point->x1 + x;
            if (point->useXOffset1 != 0) {
                vertex->x1 += xOffset;
            }
            vertex->y1 = point->y1 + y;
            if (point->useYOffset1 != 0) {
                vertex->y1 += yOffset;
            }
            count++;
            point++;
            vertex->color = color | ((point[-1].alpha * alpha) >> 7);
            vertex++;
            if (point->useXOffset0 < 0) {
                break;
            }
        } while (count != 24);
    }
    frontDrawRectangles(displayList, count, vertices, 0);
}
