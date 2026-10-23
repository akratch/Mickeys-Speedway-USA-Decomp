#ifndef OVERLAY_075_H
#define OVERLAY_075_H

#include "PR/ultratypes.h"
#include "game/math.h"

typedef struct Overlay75Vector {
    f32 x;
    f32 y;
    f32 z;
} Overlay75Vector;

typedef struct Overlay75InitData {
    u8 pad0[0xA];
    s16 angle;
    s16 scale;
    s16 slot;
    s16 height;
} Overlay75InitData;

/* The object's own header (object +0x40). */
typedef struct Overlay75Header {
    f32 scale;
    u8 pad4[0x14];
    s16 radius;
    u8 pad1A[0x3A];
    f32 xScale;
    f32 yScale;
} Overlay75Header;

/* A model's header (model +0x40); a different layout from Overlay75Header. */
typedef struct Overlay75ModelHeader {
    u8 pad00[0x0C];
    Overlay75Vector vector;
} Overlay75ModelHeader;

typedef struct Overlay75Model {
    void *entity;
    u8 pad04[4];
    s16 field08;
    s16 tableIndex;
    void *table[13];
    Overlay75ModelHeader *header;
} Overlay75Model;

typedef struct Overlay75DisplayData {
    f32 xScale;
    f32 yScale;
    u8 pad8[8];
    u8 flags;
} Overlay75DisplayData;

typedef struct Overlay75State {
    s16 slot;
    s16 active;
    s16 phase;
    s16 timer;
    f32 advanceRate;
    f32 retractRate;
    f32 baseX;
    f32 baseY;
    f32 baseZ;
    f32 endpointX;
    f32 endpointY;
    f32 endpointZ;
    f32 progress;
    f32 progressLimit;
    f32 cachedX;
    f32 cachedY;
    f32 cachedZ;
    void *effectHandle;
} Overlay75State;

typedef struct Overlay75Status {
    u8 pad00[0x61];
    u8 alternateEvent;
} Overlay75Status;

typedef struct Overlay75Object {
    s16 angle;
    u8 pad02[6];
    f32 scale;
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x10];
    f32 transitionValue;
    u8 pad2C[8];
    f32 radius;
    u8 pad38[2];
    s8 modelIndex;
    u8 pad3B[5];
    Overlay75Header *header;
    u8 pad44[4];
    Overlay75Status *status;
    Overlay75DisplayData *display;
    void *renderResource;
    u8 pad54[0x10];
    Overlay75State *state;
    Overlay75Model **models;
    u8 pad6C[0x14];
    u32 eventFlags;
} Overlay75Object;

extern f32 gOverlay75Scale;
extern s32 gOverlay75SlotFlags[];
extern f32 gOverlay75ThresholdReloc[];

/* Resident callees. */
extern void func_8005AD64(Overlay75Object *object, s32 mode, s32 index,
                          f32 argument);
extern s32 func_8005ABA8(Overlay75Object *object, f32 rate, f32 tick);
extern void func_8005AF14(Overlay75Model *model, void *entity,
                          Overlay75Object *object);
extern void func_80008128(Overlay75Object *object, f32 x, f32 y, f32 z);
extern void lightObject(Overlay75Object *object, Overlay75Model *model,
                        void *renderResource, void *tableEntry);
extern void amSndPlayXYZ(s32 id, u32 x, u32 y, u32 z,
                         s32 mode, void **output);
extern void camScreenShake(f32 x, f32 y, f32 z, f32 scale,
                           f32 lifetime);
extern void amSndSetXYZ(void *handle, u32 x, u32 y, u32 z);
extern void amSndStopXYZ(void *handle);
extern void overlay75PlayEventReloc(u16 eventId, u32 x, u32 y, u32 z, s32 mode,
                                    s32 flags);
extern void partUpdateTriggers(Overlay75Object *object, s32 updateRate);

/* Exported. */
void overlay75Init(Overlay75Object *object, Overlay75InitData *init, s32 skip);
void overlay75UpdateMovingObject(Overlay75Object *object, s32 updateRate);
void overlay75MarkSlot(s32 slot);

#endif /* OVERLAY_075_H */
