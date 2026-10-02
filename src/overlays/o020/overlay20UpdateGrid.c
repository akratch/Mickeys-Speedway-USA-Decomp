#include "ultra64.h"

typedef struct Overlay20Entry {
    f32 x;
    f32 y;
    f32 radius;
    f32 radiusSquared;
    s16 minX;
    s16 minY;
    s16 maxX;
    s16 maxY;
    s16 phase;
    s16 lifetime;
    f32 frequency;
    f32 radiusRatio;
} Overlay20Entry;

typedef struct Overlay20Vertex {
    s16 x;
    s16 value;
    s16 y;
    u8 red;
    u8 green;
    u8 blue;
    u8 pad9;
} Overlay20Vertex;

typedef struct Overlay20Grid {
    s16 minX;
    s16 baseValue;
    s16 minY;
    s16 width;
    s16 height;
    s16 columnsMinusOne;
    s16 rowsMinusOne;
    u8 pad0E[4];
    u8 bufferIndex;
    u8 baseColor;
    f32 negativeScale;
    f32 positiveScale;
    f32 displacementScale;
    Overlay20Vertex *buffers[2];
} Overlay20Grid;

extern s32 gOverlay20EntryCount;
extern Overlay20Entry *gOverlay20Entries[];
extern f32 ext_o0_6ec00(f32 value);
extern f32 func_8002A8C0(s32 angle);

/*
 * Matched (lane w2-ovld, 2026-10-02) by rewriting from the listing: one
 * index both scans the entry list and counts the vertices down (a separate
 * scan index leaves a dead `i = 0` web that takes t2 out of the temp ring),
 * the inner loop has its own index, maxX/maxY re-read the grid origin, the
 * vertex buffer is fetched before the vertex count, and the overlap array is
 * sized for the 32 entries overlay20ConfigureEntry allows, declared after the
 * scalars (two of which are unused) so it lands at sp+0x6C in the 0x140 frame.
 */
void overlay20UpdateGrid(Overlay20Grid *grid) {
    Overlay20Entry *entry;
    Overlay20Vertex *vertex;
    s32 minX;
    s32 minY;
    s32 maxX;
    s32 maxY;
    s32 pad0;
    s32 overlapCount;
    s32 i;
    s32 vertexX;
    s32 vertexY;
    f32 total;
    f32 dx;
    f32 dy;
    f32 distanceSquared;
    f32 distance;
    f32 amplitude;
    f32 output;
    s32 color;
    s32 pad1;
    s32 j;
    Overlay20Entry *overlaps[32];

    grid->bufferIndex ^= 1;
    minX = grid->minX;
    minY = grid->minY;
    maxX = grid->minX + grid->width;
    maxY = grid->minY + grid->height;
    overlapCount = 0;
    for (i = 0; i < gOverlay20EntryCount; i++) {
        entry = gOverlay20Entries[i];
        if (entry != NULL && entry->minX <= maxX && entry->minY <= maxY &&
            entry->maxX >= minX && entry->maxY >= minY) {
            overlaps[overlapCount++] = entry;
        }
    }

    vertex = grid->buffers[grid->bufferIndex];
    i = (grid->columnsMinusOne + 1) * (grid->rowsMinusOne + 1);
    if (overlapCount == 0) {
        while (i--) {
            vertex->value = grid->baseValue;
            vertex->red = grid->baseColor;
            vertex->green = grid->baseColor;
            vertex->blue = grid->baseColor;
            vertex++;
        }
    } else {
        while (i--) {
            vertexX = vertex->x;
            vertexY = vertex->y;
            total = 0.0f;
            for (j = 0; j < overlapCount; j++) {
                entry = overlaps[j];
                if (entry->minX < vertexX && entry->minY < vertexY &&
                    vertexX < entry->maxX && vertexY < entry->maxY) {
                    dx = vertexX - entry->x;
                    dy = vertexY - entry->y;
                    distanceSquared = dx * dx + dy * dy;
                    if (distanceSquared < entry->radiusSquared) {
                        distance = ext_o0_6ec00(distanceSquared);
                        amplitude = (entry->radius - distance) * entry->radiusRatio;
                        total += func_8002A8C0(entry->phase + (s32)(entry->frequency * distance)) * amplitude;
                    }
                }
            }
            output = grid->displacementScale * total;
            if (total < 0.0f) {
                if (output <= -1.0f) {
                    output = -grid->negativeScale;
                } else {
                    output *= grid->negativeScale;
                }
            } else if (output >= 1.0f) {
                output = grid->positiveScale;
            } else {
                output *= grid->positiveScale;
            }
            color = grid->baseColor + (s32)output;
            vertex->value = grid->baseValue + (s32)total;
            vertex->red = color;
            vertex->green = color;
            vertex->blue = color;
            vertex++;
        }
    }
}
