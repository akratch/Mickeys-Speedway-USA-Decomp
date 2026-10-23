#ifndef OVERLAY_035_H
#define OVERLAY_035_H

#include "ultra64.h"

/* The level header, as the initializer (mode and texture fields) and the
 * model loader (allocation counts and the light arguments) read it. */
typedef struct O35Level {
    u8 pad0[0x82];
    u8 flag82;
    s8 mode83;
    u8 pad84[0xE];
    s16 handle92;
    u8 pad94[0x5A];
    s16 allocationCount;
    s16 segmentAllocationCount;
    u8 padF2[4];
    s8 flagF6;
    u8 padF7[0x12];
    u8 finalArg0;
    u8 finalArg1;
} O35Level;

typedef struct O35TextureInfo {
    void *texture;
    u8 pad4[4];
} O35TextureInfo;

typedef struct O35Segment {
    u8 *vertices;
    void *triangles;
    void *unk8;
    void *groups;
    u32 *masks;
    void *collisionFacets;
    void *unk18;
    f32 *collisionPlanes;
    s16 count20;
    s16 dataOffset;
    u8 pad24[0xA];
    u8 flag2E;
    u8 pad2F;
    void *colorData;
    u8 pad34[4];
    s32 selectedValue;
    u8 pad3C[4];
} O35Segment;

typedef struct O35Bounds {
    s16 x0;
    s16 y0;
    s16 z0;
    s16 x1;
    s16 y1;
    s16 z1;
} O35Bounds;

typedef struct O35Model {
    O35TextureInfo *textures;
    O35Segment *segments;
    O35Bounds *bounds;
    void *unkC;
    void *unk10;
    void *unk14;
    s16 textureCount;
    s16 segmentCount;
    u8 pad1C[0x10];
    s32 modelSize;
} O35Model;


typedef struct O35ColorData {
    u8 *colors;
    s16 *flags;
} O35ColorData;

typedef struct O35Vertex {
    s16 x;
    s16 y;
    s16 z;
    u8 pad6[4];
} O35Vertex;

typedef struct O35GridBounds {
    s16 x0;
    s16 z0;
    s16 y0;
    s16 x1;
    s16 y1;
    s16 z1;
} O35GridBounds;

typedef struct O35GridSource {
    O35Vertex *vertices;
    u8 *indices;
    u8 pad8[4];
    u8 *groups;
    u32 *masks;
    u8 *zMasks;
    u8 pad18[0xC];
    s16 groupCount;
} O35GridSource;

typedef struct O35CollisionVertex {
    s16 x;
    s16 y;
    s16 z;
    u8 pad06[4];
} O35CollisionVertex;

typedef struct O35CollisionTriangle {
    u8 flags;
    u8 selectors[3];
    u8 pad04[0xC];
} O35CollisionTriangle;

typedef struct O35CollisionSpan {
    u8 pad00[6];
    s16 vertexBase;
    s16 triangleStart;
    u8 pad0A[2];
    u32 flags;
} O35CollisionSpan;

typedef struct O35CollisionRecord {
    u16 plane;
    u16 edgeNeighbor[3];
} O35CollisionRecord;

typedef struct O35CollisionPlane {
    f32 x;
    f32 y;
    f32 z;
    f32 d;
} O35CollisionPlane;

typedef struct O35CollisionSegment {
    O35CollisionVertex *vertices;
    O35CollisionTriangle *triangles;
    void *unk08;
    O35CollisionSpan *spans;
    void *unk10;
    void *unk14;
    O35CollisionRecord *records;
    f32 *planes;
    u8 pad20[2];
    s16 triangleCount;
    s16 spanCount;
} O35CollisionSegment;

typedef struct {
    u8 pad0[2];
    s16 value;
    u8 pad4[6];
} Overlay35Value;

typedef struct {
    u8 pad0[6];
    s16 valueIndex;
    u8 pad8[4];
    u32 flags;
} Overlay35Record;

typedef struct {
    Overlay35Value *values;
    u8 pad4[8];
    Overlay35Record *records;
    u8 pad10[0x14];
    s16 count;
    u8 pad26[0x16];
    s16 selectedValue;
} Overlay35State;

extern O35Level *D_800792EC;
extern O35Model *D_800792E8;
extern void *D_800C9548;
extern s32 D_800C9544;
extern void *D_800C99BC;
extern s32 D_800C9560;
extern s32 D_800C9568;
extern void *D_800792F0;
extern s32 D_800792F4;
extern u8 D_80079274;
extern s32 D_800C9D38;
extern void **D_800C9D2C;
extern s16 *D_800C9D30;
extern s16 *D_800C9D34;
extern void *D_800C9D20;
extern s32 D_800C9D3C;
extern void *D_800C95A8;
extern s32 *gOverlay35ModelTable;
extern s32 D_80079314;
extern s32 gOverlay35SkipCollisionEdges;

extern void *mmAlloc(s32 size, s32 tag);
extern void mmFree(void *value);
extern O35Level *levelGetLevel(void);
extern s32 mainGetNumberOfCameras(void);
extern void camSetMode(s32);
extern void func_80004C28(s32, s32);
extern void animseqLoadLevelData(s32);
extern void func_80004FE0(s32);
extern void shadowInitBuffers(void *, void *, s32, s32, s32);
extern void shadowGenerate(s32, s32);
extern void shadowChangeBuffer(void);
extern void modelSetModelFlags(s32);
extern void func_8000C540(s32);
extern void *texLoadTexture(s32);
extern void overlay98CollectUniqueY(void *);
extern void func_8002A244(void);
extern void mainPreNMI(void);
extern s32 *piRomLoad(s32);
extern void piRomLoadSection(s32, void *, s32, s32);
extern void func_8004D7E0(void *, void *);
extern void *align4(void *);
extern void *align16(void *);
extern s32 mmGetDelay(void);
extern void mmSetDelay(s32);
extern void *mmAllocAtAddr(s32, void *, s32);
extern void trackLightAllocate(u8, u8);
extern f32 sqrtf(f32 value);

#endif
