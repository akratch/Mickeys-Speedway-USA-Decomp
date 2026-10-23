#ifndef OVERLAYS_OVERLAY_065_H
#define OVERLAYS_OVERLAY_065_H

#include "PR/ultratypes.h"

typedef struct Overlay65TrailRecord {
    f32 x[9];
    f32 y[9];
    f32 z[9];
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
    f32 floorY;
    u8 active;
    u8 red;
    u8 green;
    u8 blue;
} Overlay65TrailRecord;

#endif
