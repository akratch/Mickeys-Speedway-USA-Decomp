#ifndef OVERLAY_099_H
#define OVERLAY_099_H

#include "PR/ultratypes.h"
#include "game/framebuffer_state.h"
#include "game/math.h"
#include "ultra64.h"
#include "n_audio/gbi.h"
typedef struct Overlay99Vec3 {
    f32 x;
    f32 y;
    f32 z;
} Overlay99Vec3;

typedef struct Overlay99RenderEntry {
    u8 *spriteData;
    s8 tableIndex;
    u8 pad05[3];
    f32 scale;
    u8 pad0C[8];
} Overlay99RenderEntry;

typedef struct Overlay99TableOwner {
    u8 pad00[0x40];
    Overlay99Vec3 *vectors;
} Overlay99TableOwner;

typedef struct Overlay99RenderState {
    s16 rotation0;
    s16 rotation1;
    s16 heading;
    u16 flags;
    f32 transformScale;
    f32 x;
    f32 y;
    f32 z;
    u8 pad18[0x21];
    u8 mode39;
    u8 pad3A[6];
    f32 *unitScale;
    u8 pad44[0x0C];
    f32 *opacity;
    u8 pad54[0x0C];
    Overlay99RenderEntry *entries;
    u8 pad64[4];
    Overlay99TableOwner **tableOwner;
    u8 pad6C[0x20];
    u8 entryCount;
} Overlay99RenderState;

typedef struct Overlay99CameraSprite {
    s16 angle;
    s16 frame;
    u16 pad04;
    u16 divisor;
    f32 transformScale;
    f32 matrixScale;
    f32 x;
    f32 y;
    f32 z;
    s32 frameCount;
    u8 *spriteData;
} Overlay99CameraSprite;

typedef struct Overlay99Object Overlay99Object;

typedef struct Overlay99Entry {
    f32 value00;
    f32 value04;
    f32 value08;
    f32 value0C;
    f32 value10;
    f32 value14;
    f32 value18;
    f32 value1C;
    f32 value20;
    f32 value24;
    s16 kind28;
    s8 mode2A;
    s8 spawnMode2B;
    Overlay99Object *object2C;
} Overlay99Entry;

typedef struct Overlay99Storage {
    Overlay99Entry entries[3];
    s32 count;
} Overlay99Storage;

typedef struct Overlay99SpawnDescriptor {
    s16 kind0;
    u8 reserved2[2];
    s16 x4;
    s16 y6;
    s16 height8;
    s8 modeA;
    u8 flagsB;
    u8 zeroC;
    u8 zeroD;
} Overlay99SpawnDescriptor;

struct Overlay99Object {
    u8 pad00[6];
    s16 flags6;
    u8 pad08[0x34];
    void *owner3C;
};

typedef struct Overlay99Influence {
    f32 x0;
    f32 z0;
    f32 x1;
    f32 z1;
    f32 longitudinalScale;
    f32 widthScale;
    f32 edgeWidth;
    f32 intensity;
    f32 angleDegrees;
    f32 angleScale;
} Overlay99Influence;

/* Overlay 99 +0x800: sorted translucent-entry render pass. */

typedef struct Overlay99Transform {
    u8 pad00[0x0C];
    f32 x;
    f32 y;
    f32 z;
} Overlay99Transform;

typedef struct Overlay99Segment {
    f32 x0;
    f32 z0;
    f32 x1;
    f32 z1;
    u8 pad10[0x1A];
    s8 headingOffset;
    u8 pad2B;
    Overlay99RenderState *object;
} Overlay99Segment;

/*
 * Overlay 99 +0xDDC: a full-screen pass that uses the framebuffer as its
 * texture.  Each grid cell loads one tile of it and draws it through two
 * three-vertex rows of the current vertex grid.
 *
 * Matched 2026-10-02 by rewriting the inherited m2c shape from the listing:
 * one packet macro per command on (*displayList)++, the tile corners as
 * MIN/MAX expressions, the two vertex rows indexed from the grid
 * (`i * width + j`, `(i + 1) * width + j`) so uopt builds the two offsets
 * and their +20 steps, and the vertex address held in a local so the macro
 * reads the buffer table once per command.  Both loops carry their second
 * induction in the for-header (`i++, y += stepY`; `j += 2, x += stepX`),
 * which is what orders the latch loads.  The frame's last cell is one
 * unused local.  The inherited -Wo,-r4300_mul override was inert and is
 * removed.
 *
 * PROVENANCE: no code adapted.  The vertex and polygon packet encodings
 * follow Jet Force Gemini's public f3ddkr.h (gSPVertexJFG, gSPPolygon),
 * with this game's vertex-slot field in bits 9-15 of the length word.
 */
typedef struct Overlay99Vertex {
    s16 x;
    s16 y;
    s16 z;
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} Overlay99Vertex;


extern Overlay99Entry gOverlay99Entries[];

extern void func_80006EA0(void *resource);

extern Overlay99Storage gOverlay99Storage;

extern void func_80037414(s32 mode, f32 x, f32 y, s32 zero0,
                                         s32 zero1, s32 zero2, s32 zero3);

extern Overlay99Object *func_8000590C(
    Overlay99SpawnDescriptor *descriptor, s32 one);

extern void viFrameRateReset(void);

extern f32 sqrtf(f32 value);

extern s32 D_8007BE84;

extern s32 gOverlay99GridWidth;

extern s32 gOverlay99GridHeight;

extern s32 gOverlay99WidthMinusOne;

extern s32 gOverlay99HeightMinusOne;

extern s32 gOverlay99Arg4;

extern s32 gOverlay99Arg5;



extern s32 gOverlay99SegmentCount;

extern Overlay99Segment gOverlay99Segments[];

extern f32 camGetProjZ(f32 x, f32 y, f32 z);

extern Overlay99Transform *camGetPtr(void);

extern void func_80022E80(Overlay99RenderState *state);

extern void func_8002AA50(Overlay99RenderState *state, MtxF matrix);

extern void mtxf_transform_point(MtxF matrix, f32 x, f32 y, f32 z,
                                             f32 *outX, f32 *outY, f32 *outZ);

extern void func_80022FD4(Gfx **displayList, Mtx **matrices,
                                       void *vertices,
                                       Overlay99RenderState *state,
                                       f32 *opacity,
                                       Overlay99CameraSprite *sprite,
                                       s32 mode, s32 selector);

extern f32 gOverlay99IntensityScale;

extern f32 gOverlay99TransformZ;

extern void viGetCurrentSize(s32 *width, s32 *height);

extern void rcpClearZBuffer(Gfx **displayList, u32 width,
                                       u32 height, s32 x1, s32 y1, s32 x2,
                                       s32 y2);

extern void texDPInit(Gfx **displayList);


extern f32 sqrtf(f32 squared);

extern void func_80009E78(Gfx **displayList, Mtx **matrices,
                                       void *vertices,
                                       Overlay99RenderState *object);

extern void *D_8007BE80;

extern Overlay99Vertex *D_8007BE88[];


extern u8 gO99TrianglesReloc[];

extern void camStandardPersp(Gfx **displayList, Mtx **matrices);

extern void overlay99RenderSegments(Gfx **displayList, Mtx **matrices,
                                         void *vertices, f32 scale);

#endif
