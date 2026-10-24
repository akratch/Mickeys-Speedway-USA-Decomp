#ifndef OVERLAY_021_H
#define OVERLAY_021_H

#include "PR/ultratypes.h"
#include "game/math.h"

typedef struct Overlay21Plane {
    s16 id;
    s8 lowerPriority;
    s8 upperPriority;
    f32 normalX;
    f32 normalY;
    f32 normalZ;
    f32 distance;
} Overlay21Plane;

typedef struct Overlay21Object {
    s16 angle0;
    s16 angle1;
    s16 unused4;
    u8 pad6[6];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x16];
    s16 id;
    u8 pad30[0x34];
    Overlay21Plane *plane;
} Overlay21Object;

typedef struct Overlay21Position {
    u8 pad0[0xC];
    f32 x;
    f32 y;
    f32 z;
} Overlay21Position;

typedef struct Overlay21PriorityEntry {
    u8 pad0[2];
    s16 priority;
    Overlay21Position *position;
} Overlay21PriorityEntry;

typedef struct Overlay21Init {
    u8 pad0[0xA];
    u8 angle0;
    u8 angle1;
    u8 lowerPriority;
    u8 upperPriority;
} Overlay21Init;

extern s32 gOverlay21ObjectCount;
extern Overlay21Object *gOverlay21Objects[];

Overlay21Position *camGetPtr(void);

#endif
