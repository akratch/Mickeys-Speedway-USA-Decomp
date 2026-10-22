#ifndef OVERLAY_019_H
#define OVERLAY_019_H

#include "PR/ultratypes.h"

typedef struct O19Vertex {
    s16 x;
    s16 y;
    s16 z;
    u8 unknown06[4];
} O19Vertex;

typedef struct O19Point {
    u8 unknown00;
    u8 selectors[3];
    u8 unknown04[12];
} O19Point;

typedef struct O19Span {
    u8 unknown00[6];
    s16 vertexBase;
    s16 itemStart;
    u8 unknown0A[2];
    u32 flags;
} O19Span;

typedef struct O19Context {
    u8 unknown00[4];
    O19Vertex *vertices;
} O19Context;

typedef struct O19Group {
    u8 unknown00[0x14];
    s16 itemCount;
    s16 spanCount;
    u8 unknown18[8];
    O19Point *points;
    O19Span *spans;
    u8 unknown28[0x14];
    s16 xLower;
    s16 yLower;
    s16 zLower;
    s16 xUpper;
    s16 yUpper;
    s16 zUpper;
} O19Group;

typedef struct O19AdjacencyRecord {
    u16 item;
    u16 edgeNeighbor[3];
} O19AdjacencyRecord;

typedef struct O19Output {
    O19AdjacencyRecord *records;
    u32 *masks;
    f32 *planes;
} O19Output;

typedef struct Overlay19Header {
    u8 pad0[0x22];
    s8 count;
    u8 pad23[0x4E];
    s8 selected;
} Overlay19Header;

typedef struct Overlay19Object {
    u8 pad0[0x40];
    Overlay19Header *header;
    u8 pad44[0x24];
    void **items;
} Overlay19Object;

typedef struct O19BuildGroup {
    u8 pad00[0x14];
    s16 itemCount;
    u8 pad16[0x38];
    s8 gate4E;
} O19BuildGroup;

extern void *mmAlloc2(s32 size, s32 tag);
extern void mmFree(void *value);
extern void *mmGetDelay(void);
extern void mmSetDelay(void *value);
extern void mmAllocAtAddr(s32 size, void *source, s32 tag);
extern f32 sqrtf(f32 value);

#endif
