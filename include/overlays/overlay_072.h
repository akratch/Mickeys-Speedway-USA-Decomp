#ifndef OVERLAY_072_H
#define OVERLAY_072_H

#include "PR/ultratypes.h"

typedef struct Overlay72Component {
    f32 scale;
    f32 pairedScale;
} Overlay72Component;

typedef struct Overlay72State {
    f32 scale;
    f32 y;
    f32 z;
} Overlay72State;

typedef struct Overlay72InitObject {
    s16 angle;
    u8 pad2[4];
    s16 flags;
    u8 pad8[4];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x34];
    Overlay72Component *component;
    u8 pad50[0x14];
    Overlay72State *state;
} Overlay72InitObject;

typedef struct Overlay72Config {
    u8 pad0[0x0A];
    u8 angle;
    u8 scale;
    s8 yOffset;
    s8 zOffset;
} Overlay72Config;

typedef struct Overlay72Candidate {
    u8 pad0[0x10];
    f32 height;
} Overlay72Candidate;

typedef struct Overlay72Bounds {
    s32 queryType;
    f32 minimum;
    f32 maximum;
} Overlay72Bounds;

typedef struct Overlay72QueryObject {
    u8 pad0[0x0C];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x4C];
    Overlay72Bounds *bounds;
} Overlay72QueryObject;

extern const f32 gOverlay72ComponentScale;

s32 func_8005776C(f32 x, f32 y, f32 z, s32 queryType,
                        s32 includeInactive, Overlay72Candidate **results);
void func_overlay_008_F0000F1C_185EC74(Overlay72Candidate *candidate, s32 value);

void overlay72Init(Overlay72InitObject *object, Overlay72Config *config);
void overlay72Update(Overlay72QueryObject *object, s32 updateRate);

#endif
