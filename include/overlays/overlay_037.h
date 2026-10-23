#ifndef OVERLAY_037_H
#define OVERLAY_037_H

#include "PR/ultratypes.h"

/*
 * Overlay 37: one translation unit (ADR 0006). The per-function files' private
 * views collapse into the structures below: an effect object, its state block,
 * and the resource block a target object carries in its state slot. Both
 * local DKR v77/v80 object and semantic-signature scans are negative for every
 * function in this module, and the pinned JFG scan has no exact source donor.
 */

typedef struct Overlay37Resource {
    s8 selector;
    u8 pad001[0x1A7];
    u16 flags1A8;
    u8 pad1AA[0x292];
    u8 effectSource[0x0C];
    f32 x448;
    f32 y44C;
    f32 z450;
} Overlay37Resource;

typedef struct Overlay37State Overlay37State;

/* A target is an object of the same layout whose state slot holds its
 * Overlay37Resource rather than an Overlay37State. */
typedef struct Overlay37Object {
    u8 pad00[4];
    s16 wave;
    u8 pad06[6];
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x16];
    s16 angle;
    u8 pad30[0x34];
    Overlay37State *state;
    void **resource;
} Overlay37Object;

struct Overlay37State {
    f32 x;
    f32 y;
    f32 z;
    f32 phase;
    Overlay37Object *target;
};

typedef struct Overlay37Init {
    u8 pad00[4];
    s16 x;
    s16 y;
    s16 z;
    u8 pad0A[2];
    Overlay37Object *target;
} Overlay37Init;

/* One per resource selector: whether a distance was recorded this frame,
 * and the smallest one. */
typedef struct Overlay37Record {
    s32 active;
    f32 distance;
} Overlay37Record;

typedef struct Overlay37Command {
    u32 w0;
    u32 w1;
} Overlay37Command;

typedef struct Overlay37Camera {
    s16 angle0;
    s16 angle2;
    u8 pad04[8];
    f32 x;
    f32 y;
    f32 z;
} Overlay37Camera;

typedef struct Overlay37Position {
    f32 x;
    f32 y;
    f32 z;
} Overlay37Position;

typedef struct Overlay37Transform {
    s16 angle0;
    s16 angle2;
    s16 objectAngle;
    u8 pad06[2];
    f32 scale;
    Overlay37Position position;
} Overlay37Transform;

extern f32 gOverlay37Period;
extern Overlay37Record gOverlay37Records[4];
extern u8 gOverlay37DisplayData[];
extern u8 gOverlay37DisplayData78[];

extern Overlay37Camera *camGetPtr(void);
extern s32 func_80021964(void);
extern f32 func_8002A8C0(s32 angle);
extern void func_80006EA0(Overlay37Object *object);
extern void func_8002A250(s32 mode, void *source, Overlay37State *state,
                          Overlay37Position *position);
extern void func_800244EC(Overlay37Command **commands, void *renderContext,
                          Overlay37Transform *transform, f32 scale,
                          f32 extra);
extern void texDPTextureX(Overlay37Command **commands, void *resource,
                          s32 mode, s32 flags);
extern void func_8002460C(Overlay37Command **commands);

void overlay37Init(Overlay37Object *object, Overlay37Init *init);
void overlay37Update(Overlay37Object *object, s32 ticks);
void overlay37RenderEffect(Overlay37Command **commands, void *renderContext,
                           Overlay37Object *object);
void overlay37RecordMinimum(s32 index, f32 value);
s32 overlay37RecordActive(s32 index);

#endif
