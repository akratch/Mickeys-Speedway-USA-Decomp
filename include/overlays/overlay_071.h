#ifndef OVERLAY_071_H
#define OVERLAY_071_H

#include "PR/ultratypes.h"

/* Overlay 71: a two-bank coloured quad effect. */

typedef struct O71Input {
    u8 pad00[0xA];
    u16 value0A;
    u16 value0C;
    s16 value0E;
    s16 value10;
    s16 value12;
    u8 value14;
    u8 value15;
    u8 value16;
    u8 value17;
    u8 value18;
    u8 value19;
    u16 value1A;
    u8 value1C;
    u8 value1D;
    u8 value1E;
    s8 value1F;
    s16 value20;
    s16 value22;
} O71Input;

typedef struct O71Vec3 {
    f32 x;
    f32 y;
    f32 z;
} O71Vec3;

typedef struct O71Vertex {
    s16 x;
    s16 y;
    s16 z;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} O71Vertex;

/* The effect's state block: two eight-vertex banks, then the parameters the
 * initializer converts from the spawn record. */
typedef struct O71State {
    O71Vertex vertices[2][8];
    f32 centerX;
    f32 centerY;
    f32 centerZ;
    f32 extentX;
    f32 extentY;
    f32 radius;
    f32 scaleX;
    f32 scaleY;
    f32 lowerLimit;
    f32 upperLimit;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
    u8 flags;
    u8 bank;
    u16 status;
} O71State;

typedef struct O71Object {
    s16 value00;
    s16 value02;
    s16 value04;
    u8 pad06[2];
    f32 value08;
    u8 pad0C[0x58];
    O71State *state;
    s32 *resourceIndex;
    u8 pad6C[0x18];
    s32 active;
} O71Object;

typedef struct Overlay71Command {
    u32 w0;
    u32 w1;
} Overlay71Command;

typedef struct Overlay71SourceCoordinate {
    s16 x;
    s16 y;
    s16 z;
} Overlay71SourceCoordinate;

typedef struct Overlay71CoordinateRecord {
    u8 pad0[6];
    s16 x;
    u8 pad8[2];
    s16 y;
    u8 padC[2];
    s16 z;
} Overlay71CoordinateRecord;

/* Module data, still owned by the raw image (phase 3). */
extern const f32 gOverlay71Scale0;
extern const f32 gOverlay71Scale1;
extern s32 gOverlay71InitialResourceReloc;
extern s16 gOverlay71Offset;
extern Overlay71CoordinateRecord gOverlay71PrimaryRecords[];
extern Overlay71CoordinateRecord gOverlay71SecondaryRecords[];
extern Overlay71SourceCoordinate gOverlay71PrimaryCoordinates[];
extern Overlay71SourceCoordinate gOverlay71SecondaryCoordinates[];
extern u8 D_80000008[];
extern u8 D_80000028[];
extern u8 D_800000D8[];

/* Resident data and functions. */
extern u8 D_80079274;
extern void SetLinkSlot(s32 slot, u16 tag, u16 useCount);
extern f32 sqrtf(f32 value);
extern void camPushModelMtx(Overlay71Command **commands, void *context,
                            O71Object *object, f32 scale, f32 extra);
extern void texDPTextureX(Overlay71Command **commands, s32 resource, s32 mode,
                          s32 flags);
extern void camPopModelMtx(Overlay71Command **commands);

/* This module's exports (ORT rows). */
void func_overlay_071_F0000000_18C9B20(O71Object *object, O71Input *input,
                                       s32 preserveState);
void func_overlay_071_F0000278_18C9D98(O71Object *object);
void overlay71UpdateCoordinates(s32 amount);
void func_overlay_071_F0000870_18CA390(Overlay71Command **commands,
                                       void *context, O71Object *object);

#endif
