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

#ifndef OVERLAY20_OVERLAP_CAPACITY
#define OVERLAY20_OVERLAP_CAPACITY 50
#endif
#ifdef REGISTER_LOCALS
#define O20_REGISTER register
#else
#define O20_REGISTER
#endif

/*
 * The overlap pointer is not a separate local: indexing the array directly
 * avoids the spill, and dx/dy/amplitude are written inline. Declared homes
 * still pad the frame to 0x170 against the target 0x140. register, a
 * scan-only pointer, and dropping minX/minY do not close it.
 */
#ifdef NON_MATCHING
void overlay20UpdateGrid(Overlay20Grid *grid) {
    Overlay20Entry *overlaps[OVERLAY20_OVERLAP_CAPACITY];
#ifndef SCOPED_LOCALS
    O20_REGISTER Overlay20Entry **cursor;
    O20_REGISTER Overlay20Entry **end;
    O20_REGISTER Overlay20Entry **overlapCursor;
    O20_REGISTER Overlay20Entry *entry;
    O20_REGISTER Overlay20Vertex *vertex;
    O20_REGISTER s16 minX;
    O20_REGISTER s16 minY;
#ifdef EXPLICIT_BOUNDS
    O20_REGISTER s32 maxX;
    O20_REGISTER s32 maxY;
    O20_REGISTER s32 entryCount;
#endif
    O20_REGISTER s16 vertexX;
    O20_REGISTER s16 vertexY;
    O20_REGISTER s32 overlapCount;
    O20_REGISTER s32 remaining;
    O20_REGISTER s32 overlapIndex;
    O20_REGISTER f32 total;
    O20_REGISTER f32 distanceSquared;
    O20_REGISTER f32 distance;
    O20_REGISTER f32 output;
    O20_REGISTER s8 color;
#else
    O20_REGISTER s32 overlapCount;
#endif

#ifdef SCOPED_LOCALS
    {
    O20_REGISTER Overlay20Entry **cursor;
    O20_REGISTER Overlay20Entry **end;
    O20_REGISTER Overlay20Entry *entry;
    O20_REGISTER s16 minX;
    O20_REGISTER s16 minY;
#ifdef EXPLICIT_BOUNDS
    O20_REGISTER s32 maxX;
    O20_REGISTER s32 maxY;
    O20_REGISTER s32 entryCount;
#endif
#endif
    grid->bufferIndex ^= 1;
#ifdef EXPLICIT_BOUNDS
    entryCount = gOverlay20EntryCount;
#endif
    minX = grid->minX;
    minY = grid->minY;
    overlapCount = 0;

#ifdef EXPLICIT_BOUNDS
    maxX = minX + grid->width;
    maxY = minY + grid->height;
    if (entryCount > 0) {
#else
    if (gOverlay20EntryCount > 0) {
#endif
        cursor = gOverlay20Entries;
#ifdef EXPLICIT_BOUNDS
        end = cursor + entryCount;
#else
        end = cursor + gOverlay20EntryCount;
#endif
#ifndef SCAN_TOP_LOAD
        entry = *cursor;
#endif
        do {
#ifdef SCAN_TOP_LOAD
            entry = *cursor;
#endif
            cursor++;
#ifdef EXPLICIT_BOUNDS
            if (entry != 0 && maxX >= entry->minX && maxY >= entry->minY &&
#else
            if (entry != 0 && minX + grid->width >= entry->minX &&
                minY + grid->height >= entry->minY &&
#endif
                entry->maxX >= minX && entry->maxY >= minY) {
                overlaps[overlapCount++] = entry;
            }
#if defined(EXPLICIT_BOUNDS) && !defined(SCAN_TOP_LOAD)
            entry = *cursor;
#elif !defined(EXPLICIT_BOUNDS) && !defined(SCAN_TOP_LOAD)
            if (cursor < end) {
                entry = *cursor;
            }
#endif
        } while (cursor < end);
    }
#ifdef SCOPED_LOCALS
    }
    {
    O20_REGISTER Overlay20Entry **overlapCursor;
    O20_REGISTER Overlay20Entry *entry;
    O20_REGISTER Overlay20Vertex *vertex;
    O20_REGISTER s16 vertexX;
    O20_REGISTER s16 vertexY;
    O20_REGISTER s32 remaining;
    O20_REGISTER s32 overlapIndex;
    O20_REGISTER f32 total;
    O20_REGISTER f32 distanceSquared;
    O20_REGISTER f32 distance;
    O20_REGISTER f32 amplitude;
    O20_REGISTER f32 output;
    O20_REGISTER s8 color;
#endif

    remaining = (grid->columnsMinusOne + 1) * (grid->rowsMinusOne + 1);
    vertex = grid->buffers[grid->bufferIndex];

    if (overlapCount == 0) {
        while (remaining-- != 0) {
            vertex++;
            vertex[-1].value = grid->baseValue;
            vertex[-1].red = grid->baseColor;
            vertex[-1].green = grid->baseColor;
            vertex[-1].blue = grid->baseColor;
        }
    } else {
        while (remaining-- != 0) {
            vertexX = vertex->x;
            vertexY = vertex->y;
            total = 0.0f;
            overlapIndex = 0;
            if (overlapCount > 0) {
                overlapCursor = overlaps;
                do {
                    entry = *overlapCursor;
                    if (entry->minX < vertexX && entry->minY < vertexY &&
                        vertexX < entry->maxX && vertexY < entry->maxY) {
                        distanceSquared = ((f32)vertexX - entry->x) * ((f32)vertexX - entry->x) +
                            ((f32)vertexY - entry->y) * ((f32)vertexY - entry->y);
                        if (distanceSquared < entry->radiusSquared) {
                            distance = ext_o0_6ec00(distanceSquared);
                            total += func_8002A8C0(entry->phase + (s32)(entry->frequency * distance)) *
                                ((entry->radius - distance) * entry->radiusRatio);
                        }
                    }
                    overlapIndex++;
                    overlapCursor++;
                } while (overlapIndex != overlapCount);
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
            vertex++;
            vertex[-1].value = grid->baseValue + (s32)total;
            vertex[-1].red = color;
            vertex[-1].green = color;
            vertex[-1].blue = color;
        }
    }
#ifdef SCOPED_LOCALS
    }
#endif
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o020/overlay20UpdateGrid/func_overlay_020_F0000A68_1877040.s")
#endif

/* PLATEAU-HANDOFF:overlay20UpdateGrid:start
 * symbol: overlay20UpdateGrid
 * score: 173 differing words
 * frame: 0x170
 * relocations: 6
 * first-mismatch: +0x0
 * summary: 173 words, delta 0, frame 0x170. missing-CSE overlapBase spill at entryCount test; dx/dy/amplitude inlines held size, minX/minY and pointer restore regressed.
 * PLATEAU-HANDOFF:overlay20UpdateGrid:end
 */
