#ifndef OVERLAY_081_H
#define OVERLAY_081_H

#include "PR/ultratypes.h"

typedef struct Overlay81Transform {
    f32 scale;
    u8 pad4[0x50];
    f32 width;
    f32 depth;
} Overlay81Transform;

typedef union Overlay81Coord {
    f32 value;
    s32 bits;
} Overlay81Coord;

typedef union Overlay81CollisionValue {
    f32 radius;
    f32 yOffset;
} Overlay81CollisionValue;

typedef struct Overlay81Collision {
    s16 flags;
    s16 active;
    Overlay81CollisionValue value;
} Overlay81Collision;

typedef struct Overlay81State {
    f32 x;
    f32 y;
    f32 z;
    s32 timer;
    s32 active;
    s32 mask;
} Overlay81State;

typedef struct Overlay81NearbyState {
    f32 radius;
    s32 index;
} Overlay81NearbyState;

typedef struct Overlay81Trigger {
    u8 pad0[0x61];
    u8 active;
} Overlay81Trigger;

typedef struct Overlay81Object {
    u8 pad0[8];
    f32 scale;
    Overlay81Coord x;
    Overlay81Coord y;
    Overlay81Coord z;
    u8 pad18[0x28];
    Overlay81Transform *transform;
    u8 pad44[4];
    Overlay81Trigger *trigger;
    f32 *dimensions;
    u8 pad50[0x14];
    void *state;
    u8 pad68[0x10];
    Overlay81Collision *collision;
    u8 pad7C[4];
    s32 flags;
} Overlay81Object;

typedef union Overlay81InitValueA {
    s16 scale;
    u16 radius;
} Overlay81InitValueA;

typedef union Overlay81InitValueC {
    s16 timer;
    u16 index;
} Overlay81InitValueC;

typedef struct Overlay81Init {
    u8 pad0[0xA];
    Overlay81InitValueA valueA;
    Overlay81InitValueC valueC;
    s16 bit;
} Overlay81Init;

typedef struct Overlay81NearbyObject {
    u8 pad0[0xC];
    f32 x;
    u8 pad10[4];
    f32 z;
} Overlay81NearbyObject;

void amSndPlayXYZ(s32 effectId, s32 x, s32 y, s32 z,
                               s32 mode, s32 arg5);
void partUpdateTriggers(Overlay81Object *object, s32 mode);
s32 mathRnd(s32 minimum, s32 maximum);
Overlay81NearbyObject **func_80005750(
    s32 *count, Overlay81Object *object, Overlay81NearbyState *state);

void overlay81Init(Overlay81Object *object, Overlay81Init *init, s32 unused);
void overlay81Update(Overlay81Object *object, s32 updateRate);
void overlay81SetMaskBit(s32 bit);
void overlay81InitState(Overlay81Object *object, Overlay81Init *init);
void overlay81CheckNearby(Overlay81Object *object, s32 unused);

#endif
