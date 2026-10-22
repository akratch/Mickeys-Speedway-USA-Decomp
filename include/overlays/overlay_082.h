#ifndef OVERLAY_082_H
#define OVERLAY_082_H

#include "PR/ultratypes.h"

typedef struct O82DisplayPair {
    s16 label;
    s16 value;
} O82DisplayPair;

typedef struct Overlay82State {
    s8 selection;
    s8 changed;
    u8 active;
    u8 disabled;
    s32 values[6];
    O82DisplayPair display[6];
    u16 flags;
} Overlay82State;

typedef Overlay82State O82State;
typedef struct O82Resource O82Resource;

typedef struct O82ResourceOwner {
    O82Resource *resource;
} O82ResourceOwner;

typedef struct Overlay82Object {
    u8 pad00[0x28];
    f32 progress;
    u8 pad2C[0x38];
    void *state;
    O82ResourceOwner *resourceOwner;
} Overlay82Object;

typedef Overlay82Object O82Object;

extern u32 D_800D31B8;
extern s16 D_800D31BC;
extern s16 D_800D31BE;
extern const u8 gO82DataBase[];

void amSndPlay(u16 eventId, void *nullableHandle);
void modSetTextureFrame(O82Resource *resource, s32 channel, s32 value);

void overlay82Init(Overlay82Object *object, void *entry);
void overlay82Update(O82Object *object, s32 updateRate);
s32 overlay82GetSelection(Overlay82Object *object);
u32 overlay82IsActive(Overlay82Object *object);
void overlay82Disable(Overlay82Object *object);
void overlay82Enable(Overlay82Object *object);

#endif
