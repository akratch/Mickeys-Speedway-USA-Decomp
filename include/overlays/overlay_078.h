#ifndef OVERLAY_078_H
#define OVERLAY_078_H

#include "PR/ultratypes.h"

typedef struct Overlay78Record {
    f32 value;
} Overlay78Record;

typedef struct Overlay78Object {
    s16 value;
    u8 pad2[0x62];
    Overlay78Record *record;
} Overlay78Object;

typedef struct Overlay78Config {
    u8 pad0[0x0A];
    s16 valueA;
    s16 valueC;
    s16 valueE;
} Overlay78Config;

void func_8005AD64(Overlay78Object *object, s32 arg1, s32 arg2,
                             f32 value);
void func_8005ABA8(Overlay78Object *object, f32 current, f32 target);

void overlay78Init(Overlay78Object *object, Overlay78Config *config);
void overlay78Update(Overlay78Object *object, s32 updateRate);

#endif
