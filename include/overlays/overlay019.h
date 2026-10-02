#ifndef OVERLAYS_OVERLAY019_H
#define OVERLAYS_OVERLAY019_H

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

s32 overlay19BuildPlanes(
    O19Context *context,
    O19Group *group,
    O19Output *output);

void overlay19BuildAdjacency(
    O19Context *context,
    O19Group *group,
    O19Output *output);

s32 overlay19FindAdjacent(
    O19Context *context,
    O19Group *group,
    s32 selfItem,
    s32 queryStartIndex,
    s32 queryEndIndex);

s32 overlay19ClassifyEdge(
    O19Vertex *vertices,
    s32 queryStartIndex,
    s32 queryEndIndex,
    s32 candidateStartIndex,
    s32 candidateEndIndex);

void overlay19BuildSpatialMasks(
    O19Context *context,
    O19Group *group,
    O19Output *output);

#endif
