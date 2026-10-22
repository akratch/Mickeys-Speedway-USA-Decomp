#ifndef OVERLAY_093_H
#define OVERLAY_093_H

#include "PR/ultratypes.h"

typedef struct Overlay93Config {
    u8 pad0[0x0A];
    s16 x;
    s16 y;
    s16 z;
    u8 red;
    u8 green;
    s16 arg5;
} Overlay93Config;

typedef struct Overlay93UpdateConfig {
    u8 pad0[0x0C];
    s16 x;
    s16 y;
    s16 z;
    u8 red;
    u8 green;
    s16 arg5;
} Overlay93UpdateConfig;

typedef struct Overlay93Object {
    u8 pad0[0x0C];
    f32 x;
    u8 pad10[4];
    f32 z;
    u8 pad18[0x24];
    Overlay93UpdateConfig *config;
    u8 pad40[0x44];
    f32 radiusSquared;
} Overlay93Object;

Overlay93Object *overlay93FindTargetReloc(s32 type, Overlay93Object *object);
void overlay93EmitReloc(s32 x, s32 y, s32 z, s32 red, s32 green, s32 arg5);

void overlay93Init(Overlay93Object *object, Overlay93Config *config);
void overlay93Update(Overlay93Object *object, s32 updateRate);

#endif
