#ifndef OVERLAY_076_H
#define OVERLAY_076_H

#include "PR/ultratypes.h"

typedef struct Overlay76Record {
    s32 index;
} Overlay76Record;

typedef struct Overlay76Object {
    u8 pad0[0x0C];
    s32 x;
    s32 y;
    s32 z;
    u8 pad18[0x4C];
    Overlay76Record *record;
} Overlay76Object;

extern s32 gOverlay76NextIndex;
extern s32 gOverlay76Status[8];

void overlay76SoundReloc(u16 soundId, s32 x, s32 y, s32 z, s32 arg4,
                         s32 arg5);
s32 overlay76RandomReloc(s32 min, s32 max);

void overlay76Register(Overlay76Object *object, void *entry);
void overlay76Update(Overlay76Object *object, s32 updateRate);
void overlay76TriggerRandom(void);

#endif
