#include "PR/ultratypes.h"

typedef struct Overlay101FrameRect {
    s16 x0;
    s16 y0;
    s16 x1;
    s16 y1;
    s32 color;
} Overlay101FrameRect;

extern void frontDrawRectangles(s32, s32, Overlay101FrameRect *, s32);

/* DKR v77/v80 and JFG have no exact donor for this four-rectangle builder. */
void overlay101BuildFrame(s32 kind, s32 x, s32 y, s32 width, s32 height,
                          s32 color) {
    Overlay101FrameRect *rect;
    Overlay101FrameRect batch[4];

    rect = batch;
    rect[0].x0 = x;
    rect[0].y0 = y;
    rect[0].x1 = x + 1;
    rect[0].y1 = y + height;
    rect[0].color = color;
    rect[1].x0 = x;
    rect[1].y0 = y + height - 1;
    rect[1].x1 = x + width;
    rect[1].y1 = y + height;
    rect[1].color = color;
    rect[2].x0 = x + width - 1;
    rect[2].y0 = y + 1;
    rect[2].x1 = x + width;
    rect[2].y1 = y + height - 1;
    rect[2].color = color;
    rect += 3;
    rect->x0 = x + 1;
    rect->y0 = y;
    rect->x1 = x + width;
    rect->y1 = y + 1;
    rect->color = color;

    frontDrawRectangles(kind, 4, batch, 0);
}
