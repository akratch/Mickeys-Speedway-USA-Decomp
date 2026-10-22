#ifndef OVERLAY_069_H
#define OVERLAY_069_H

#include "PR/ultratypes.h"

/* Overlay 69. Its sorted-geometry renderer is shared byte for byte with
 * overlay 88 (src/overlays/o069/overlay69DrawSortedGeometry.inc). */

typedef f32 Overlay69Matrix[4][4];

typedef struct Overlay69State {
    s16 field0, field2, field4, field6;
    s16 field8, fieldA, fieldC, fieldE;
    s32 field10, field14, field18, field1C;
    u8 pad20[0x14];
    u8 byte34, byte35, byte36, byte37;
} Overlay69State;

typedef struct Overlay69Object {
    u8 pad0[0x64];
    Overlay69State *state;
} Overlay69Object;

typedef struct Overlay69AnchorState {
    u8 reserved00[0x44];
    f32 x;
    f32 y;
    f32 z;
    s16 angle;
} Overlay69AnchorState;

typedef struct Overlay69TransformSet {
    u8 reserved00[8];
    s16 direct;
    s16 matrixIndex;
    Overlay69Matrix *matrices[1];
} Overlay69TransformSet;

typedef struct Overlay69AnchorObject {
    s16 angle;
    u8 reserved02[0xA];
    f32 x;
    f32 y;
    f32 z;
    u8 reserved18[0x4C];
    Overlay69AnchorState *anchorState;
    Overlay69TransformSet **transformSets;
    u8 reserved6C[0x27];
    u8 transformSetIndex;
} Overlay69AnchorObject;

/* Resident functions. */
extern void func_8002B040(Overlay69Matrix *matrix, f32 x, f32 y, f32 z,
                          f32 *outX, f32 *outY, f32 *outZ);
extern s32 Arctanf(f32 x, f32 z);
extern f32 func_8002A8C0(s32 angle);
extern f32 func_8002A8BC(s32 angle);

/* This module's exports (ORT rows); overlay69DrawSortedGeometry is declared
 * by the shared renderer. */
void overlay69Init(Overlay69Object *object, void *entry);
void overlay69UpdateAnchor(Overlay69AnchorObject *object, s32 updateContext);

#endif
