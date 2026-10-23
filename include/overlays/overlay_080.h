#ifndef OVERLAY_080_H
#define OVERLAY_080_H

#include "PR/ultratypes.h"

/* Overlay 80: a contact trigger. Pinned DKR v77/v80 and JFG object scans
 * found no exact donor. */

typedef struct Overlay80State {
    f32 previousDistance;
    f32 scale;
    s32 active;
} Overlay80State;

typedef struct Overlay80Notice {
    u8 pad00[4];
    u8 state;
} Overlay80Notice;

typedef struct Overlay80Candidate {
    f32 threshold;
    s32 value;
} Overlay80Candidate;

typedef struct Overlay80Object {
    s16 key;
    s16 outputB;
    s16 outputA;
    u8 pad06[2];
    f32 scaledValue;
    f32 positionX;
    f32 positionY;
    f32 positionZ;
    u8 pad18[4];
    f32 normalX;
    f32 normalY;
    f32 normalZ;
    u8 pad28[0x18];
    f32 *objectScale;
    u8 pad44[0x0C];
    Overlay80Notice *notice;
    u8 pad54[0x10];
    Overlay80State *state;
} Overlay80Object;

typedef struct Overlay80Init {
    u8 pad00[0x0A];
    s16 key;
    s16 scale;
    s16 initialValue;
} Overlay80Init;

extern s32 func_8001398C(f32 position0, f32 position1, s32 radius,
                         Overlay80Candidate ***candidatesOut);
extern void func_8001DCD0(s16 key, s32 selectedValue, s16 *outputA,
                          s16 *outputB);
extern s32 func_8005776C(f32 positionX, f32 positionY, f32 positionZ,
                         f32 radius, s32 ignoreY,
                         Overlay80Object **contactOut);
extern f32 sqrtf(f32 value);
extern void func_8005AD64(Overlay80Object *object, s32 kind, s32 value,
                          f32 amount);
extern s32 func_8005ABA8(Overlay80Object *object, f32 rate, f32 ticks);

void overlay80InitializeContact(Overlay80Object *object,
                                const Overlay80Init *init);
void overlay80UpdateContact(Overlay80Object *object, s32 ticks);

#endif
