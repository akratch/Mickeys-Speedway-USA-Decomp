#ifndef OVERLAY_090_H
#define OVERLAY_090_H

#include "PR/ultratypes.h"

typedef struct Overlay90State {
    u8 active;
    u8 flag;
    s16 angle;
    f32 x;
    f32 y;
    f32 z;
    f32 value10;
    f32 value14;
    f32 value18;
    f32 value1C;
    f32 value20;
    s16 value24;
    s16 value26;
    s16 value28;
    s16 value2A;
    s16 value2C;
    s16 value2E;
    f32 value30;
    s16 value34;
    s16 value36;
    void *value38;
    s16 value3C;
    s16 value3E;
    s16 value40;
    s16 value42;
    s16 value44;
    s16 value46;
} Overlay90State;

typedef struct Overlay90Attachment Overlay90Attachment;

typedef struct Overlay90Owner {
    s16 rotationX;
    s16 rotationY;
    s16 rotationZ;
    u8 pad06[6];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x16];
    s16 positionTag;
    u8 pad30[0x34];
    Overlay90State *state;
    Overlay90Attachment **attachment;
} Overlay90Owner;

typedef struct Overlay90Config {
    u8 pad0[4];
    s16 x;
    s16 y;
    s16 z;
    s16 angle;
} Overlay90Config;

extern f32 gOverlay90Value1C;
extern f32 gOverlay90Value20;

void func_8005AD64(Overlay90Owner *owner, s32 arg1, s32 arg2,
                          f32 arg3);
void overlay90Initialize(Overlay90Owner *owner, Overlay90Config *config);

#endif
