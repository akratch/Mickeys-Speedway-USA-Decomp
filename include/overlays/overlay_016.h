#ifndef OVERLAY_016_H
#define OVERLAY_016_H

#include "PR/ultratypes.h"

typedef struct Overlay16Vertex {
    s16 x;
    s16 y;
    s16 z;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} Overlay16Vertex;

typedef struct Overlay16ColorSource {
    u8 *colors;
    s16 *blocks;
} Overlay16ColorSource;

typedef struct Overlay16Batch {
    Overlay16Vertex *vertices;
    u8 pad04[0x1C];
    s16 vertexCount;
    u8 pad22[0xC];
    u8 dirty;
    u8 pad2F;
    Overlay16ColorSource *source;
    u8 pad34[0xC];
} Overlay16Batch;

typedef struct Overlay16Context {
    u8 pad00[4];
    Overlay16Batch *batches;
    u8 pad08[0x12];
    s16 batchCount;
} Overlay16Context;

extern u8 *gOverlay16Buffer;
extern s32 gOverlay16Phase;
extern s32 gOverlay16Mode;

extern void *mmAlloc(s32 size, s32 tag);
extern void overlay16BuildGradientReloc(u8 *output, s32 red, s32 green,
                                        s32 blue, s32 endRed, s32 endGreen,
                                        s32 endBlue);
extern void mmFree(void *buffer);

#endif
