#include "overlays/overlay019.h"

/* Matched 2026-10-01 by writing the function out plainly. The previous
 * candidate emulated the target's three stack cells with a frame struct of
 * volatile unions and steered as1 with #line; all of it was standing in for
 * decisions the allocator makes by itself on ordinary locals:
 *  - the span fields are read through `group->spans[i]` itself, with no span
 *    pointer. That lets uopt strength-reduce the index; all 23 colours are
 *    then spent before the index, its reduced offset and the hoisted
 *    `flags & 0x1080` are reached, and the three stay in memory.
 *  - the six bound initialisations all sit in the else arm. With the maxima
 *    ahead of the test, the index's split piece is refused one block early
 *    and keeps a register outside the item loop.
 *  - `mask = 0` follows the vertex loop, and each bin loop initialises
 *    `binStart` in its for clause after `n`; both are colour ties decided by
 *    first occurrence.
 *  - `i` is declared sixth, which is its home in the 0x80 frame. */
void overlay19BuildSpatialMasks(O19Context *context, O19Group *group, O19Output *output) {
    s32 j;
    s32 k;
    s16 n;
    O19Point *point;
    O19Vertex *vertex;
    s32 i;
    s16 x, y, z;
    s16 xMax, xMin, yMax, yMin, zMax, zMin;
    s16 lower, upper, step, binStart, binEnd;
    u32 bit;
    u32 mask;
    s16 firstItem;
    s16 itemEnd;
    s16 vertexBase;
    u32 flags;

    for (i = 0; i < group->spanCount; i++) {
        firstItem = group->spans[i].itemStart;
        itemEnd = group->spans[i + 1].itemStart;
        vertexBase = group->spans[i].vertexBase;
        flags = group->spans[i].flags;
        for (j = firstItem; j < itemEnd; j++) {
            if (flags & 0x1080) {
                output->masks[j] = 0;
            } else {
                xMax = -32000;
                yMax = -32000;
                zMax = -32000;
                xMin = 32000;
                yMin = 32000;
                zMin = 32000;
                point = &group->points[j];
                for (k = 0; k < 3; k++) {
                    vertex = &context->vertices[point->selectors[k] + vertexBase];
                    x = vertex->x;
                    y = vertex->y;
                    z = vertex->z;
                    if (xMax < x) {
                        xMax = x;
                    }
                    if (x < xMin) {
                        xMin = x;
                    }
                    if (yMax < y) {
                        yMax = y;
                    }
                    if (y < yMin) {
                        yMin = y;
                    }
                    if (zMax < z) {
                        zMax = z;
                    }
                    if (z < zMin) {
                        zMin = z;
                    }
                }
                mask = 0;
                bit = 1;
                lower = group->xLower;
                upper = group->xUpper;
                step = ((upper - lower) >> 3) + 1;
                binEnd = step + lower;
                for (n = 0, binStart = lower; n < 8; n++) {
                    if (!(binEnd < xMin || xMax < binStart)) {
                        mask |= bit;
                    }
                    binEnd += step;
                    binStart += step;
                    bit <<= 1;
                }
                lower = group->zLower;
                upper = group->zUpper;
                step = ((upper - lower) >> 3) + 1;
                binEnd = step + lower;
                for (n = 0, binStart = lower; n < 8; n++) {
                    if (!(binEnd < zMin || zMax < binStart)) {
                        mask |= bit;
                    }
                    binEnd += step;
                    binStart += step;
                    bit <<= 1;
                }
                lower = group->yLower;
                upper = group->yUpper;
                step = ((upper - lower) >> 3) + 1;
                binEnd = step + lower;
                for (n = 0, binStart = lower; n < 8; n++) {
                    if (!(binEnd < yMin || yMax < binStart)) {
                        mask |= bit;
                    }
                    binEnd += step;
                    binStart += step;
                    bit <<= 1;
                }
                output->masks[j] = mask;
            }
        }
    }
}
