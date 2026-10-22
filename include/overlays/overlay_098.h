#ifndef OVERLAY_098_H
#define OVERLAY_098_H

#include "PR/ultratypes.h"
typedef struct Overlay98Vertex {
    u8 reserved0[2];
    s16 y;
    u8 reserved4[6];
} Overlay98Vertex;

typedef struct Overlay98PointRef {
    u8 reserved0;
    u8 vertexIndex;
    u8 reserved2[14];
} Overlay98PointRef;

typedef struct Overlay98Span {
    u8 reserved0[6];
    s16 vertexBase;
    s16 pointRefIndex;
    u8 reservedA[2];
    u32 flags;
} Overlay98Span;

typedef struct Overlay98Block {
    Overlay98Vertex *vertices;
    Overlay98PointRef *pointRefs;
    u8 reserved08[4];
    Overlay98Span *spans;
    u8 reserved10[0x14];
    s16 spanCount;
    u8 reserved26[0x1A];
} Overlay98Block;

typedef struct Overlay98Group {
    u8 reserved00[4];
    Overlay98Block *blocks;
    u8 reserved08[0x12];
    s16 blockCount;
} Overlay98Group;

extern s32 overlay98UniqueCountReloc;

extern s16 overlay98UniqueYReloc[15];


extern u8 *camGetPtr(void);

extern s32 gOverlay98AcceptedCount;

typedef struct Gfx { u32 w0, w1; } Gfx;

typedef struct O98Mtx { u8 bytes[0x40]; } O98Mtx;

typedef struct O98ModelData {
    u8 pad00[0x11];
    u8 special;
    u8 pad12[0x3C];
    s8 mode4E;
    u8 pad4F[0x19];
    void *displayListA;
    void *displayListB;
} O98ModelData;

typedef struct O98Node {
    O98ModelData *data;
    void *vertexData;
    s16 useAlternate;
    s16 partIndex;
    void *partsA[17];
    void *partsB[1];
} O98Node;

typedef struct O98Object {
    s16 rotX, rotY, rotZ;
    s16 flags;
    f32 scale;
    f32 x, y, z;
    u8 pad18[0x21];
    u8 alpha;
    s8 nodeIndex;
    u8 pad3B[5];
    s8 *stateTable;
    u8 pad44[0x24];
    void **nodes;
    u8 pad6C[0x22];
    u8 accepted;
    u8 pad8F[4];
    u8 stateIndex;
} O98Object;

typedef struct O98VisibleEntry { O98Object *object; f32 referenceY; } O98VisibleEntry;

typedef struct O98Context { u8 bytes[0x40]; } O98Context;

typedef struct O98Transform {
    s16 rot0, rot2, rot4;
    u16 pad06;
    f32 scale;
    f32 x, y, z;
} O98Transform;

extern void *camGetProjectionMtx(void);

extern void mtxf_to_mtx(void *, void *);

extern void func_8002AA50(O98Transform *, O98Mtx *);

extern void mtxf_mul(O98Mtx *, void *, O98Mtx *);

extern void func_8002AC84(O98Transform *, O98Mtx *);

extern void func_80009E78(Gfx **, u8 **, void *, O98Object *);

extern void texDPInit(Gfx **);

extern s32 gO98Toggle;

extern O98Context gO98Contexts[2];

extern O98VisibleEntry gOverlay98AcceptedEntries[0x50];

extern u8 gO98SpecialVertices[];

/* Exact O98 +0x0848 owner; DKR v77/v80 and JFG donor scans are negative. */
typedef struct Vec3f { f32 x, y, z; } Vec3f;

typedef struct O98NodeData {
    u8 pad00[0x44];
    s16 height;
} O98NodeData;

typedef struct O98HeightNode {
    O98NodeData *data;
} O98HeightNode;

extern s32 func_8001357C(f32, f32, f32 *, s32, s32);

extern s32 func_8001291C(void *, Vec3f *, Vec3f *, s32, s32);

extern s16 overlay98UniqueYReloc[];

#endif
