#include "overlays/overlay019.h"

extern void *o19AllocateReloc(s32 size, s32 tag);
extern void o19FreeReloc(void *value);
extern f32 sqrtf(f32 value);

/*
 * Matched (lane w2-ovld, 2026-10-02) as the sibling of overlay 35's
 * func_overlay_035_F0000B40_1882820, both adapted from Diddy Kong Racing's
 * collision-plane builder. Differences from the o035 copy that the listing
 * fixes: word copy of the records, 10.0f bisector scale, plane cross products
 * relative to the second vertex in both passes, no early exit. What closed it:
 * no do-while(0) macros anywhere (they split the group parameter's live range
 * at the copy loop), the edge body under `if (neighbor != 0xFFFF)` rather than
 * a `continue`, the neighbour scan as a do-while, and the unused pads and the
 * oppVertIndex position that set the frame homes.
 *
 * PROVENANCE: adapted from Diddy Kong Racing, src/tracks.c
 * (track_init_collision) and src/object_models.c (model_init_collision), by
 * way of the matched overlay 35 function.
 */
s32 overlay19BuildPlanes(O19Context *context, O19Group *group, O19Output *output) {
    s32 pad0;
    O19AdjacencyRecord *scratch;
    s32 *scratchRecord;
    s32 *record;
    s32 copyIndex;
    f32 x1, y1, z1;
    f32 x2, y2, z2;
    f32 x3, y3, z3;
    f32 nx, ny, nz;
    f32 mag;
    s32 pad1;
    f32 x5, y5, z5;
    s32 triStart;
    O19Vertex *v;
    s32 i;
    s32 triEnd;
    s32 vertexBase;
    s32 spanIndex;
    s32 j;
    s32 edge;
    s32 counter;
    s32 idx;
    s32 next;
    s32 opp;
    s32 oppVertIndex;
    s32 vertIndex;
    s32 nextVertIndex;
    s32 neighbor;
    s32 pad2;
    f32 *plane;

    scratch = o19AllocateReloc(group->itemCount * (s32)sizeof(O19AdjacencyRecord), 0x8A);
    record = (s32 *)output->records;
    scratchRecord = (s32 *)scratch;
    for (copyIndex = 0; copyIndex < group->itemCount * 2; copyIndex++) {
        *scratchRecord++ = *record++;
    }

    counter = 0;
    for (spanIndex = 0; spanIndex < group->spanCount; spanIndex++) {
        triStart = group->spans[spanIndex].itemStart;
        vertexBase = group->spans[spanIndex].vertexBase;
        triEnd = group->spans[spanIndex + 1].itemStart;
        if (group->spans[spanIndex].flags & 0x1080) {
            triStart = triEnd;
        }
        for (i = triStart; i < triEnd; i++) {
            v = &context->vertices[group->points[i].selectors[0] + vertexBase];
            x1 = v->x;
            y1 = v->y;
            z1 = v->z;
            v = &context->vertices[group->points[i].selectors[1] + vertexBase];
            x2 = v->x;
            y2 = v->y;
            z2 = v->z;
            v = &context->vertices[group->points[i].selectors[2] + vertexBase];
            x3 = v->x;
            y3 = v->y;
            z3 = v->z;
            nx = (y2 - y1) * (z3 - z2) - (z2 - z1) * (y3 - y2);
            ny = (z2 - z1) * (x3 - x2) - (x2 - x1) * (z3 - z2);
            nz = (x2 - x1) * (y3 - y2) - (y2 - y1) * (x3 - x2);
            mag = sqrtf(nx * nx + ny * ny + nz * nz);
            if (mag > 0.0f) {
                nx /= mag;
                ny /= mag;
                nz /= mag;
            }
            output->records[i].item = counter;
            output->planes[counter << 2] = nx;
            output->planes[(counter << 2) + 1] = ny;
            output->planes[(counter << 2) + 2] = nz;
            output->planes[(counter << 2) + 3] = -(x1 * nx + y1 * ny + z1 * nz);
            counter++;
        }
    }

    for (spanIndex = 0; spanIndex < group->spanCount; spanIndex++) {
        triStart = group->spans[spanIndex].itemStart;
        vertexBase = group->spans[spanIndex].vertexBase;
        triEnd = group->spans[spanIndex + 1].itemStart;
        if (group->spans[spanIndex].flags & 0x1080) {
            triStart = triEnd;
        }
        for (i = triStart; i < triEnd; i++) {
            idx = output->records[i].item * 4;
            nx = output->planes[idx + 0];
            ny = output->planes[idx + 1];
            nz = output->planes[idx + 2];
            for (edge = 0; edge < 3; edge++) {
                next = edge + 1;
                if (next >= 3) {
                    next = 0;
                }
                opp = next + 1;
                if (opp >= 3) {
                    opp = 0;
                }
                vertIndex = group->points[i].selectors[edge] + vertexBase;
                nextVertIndex = group->points[i].selectors[next] + vertexBase;
                oppVertIndex = group->points[i].selectors[opp] + vertexBase;
                neighbor = scratch[i].edgeNeighbor[edge];
                if (neighbor != 0xFFFF) {
                    if (neighbor == 0xFFFE) {
                        idx = output->records[i].item * 4;
                    } else {
                        idx = output->records[neighbor].item * 4;
                    }
                    plane = &output->planes[idx];
                    x5 = output->planes[idx + 0] + nx;
                    y5 = output->planes[idx + 1] + ny;
                    z5 = output->planes[idx + 2] + nz;
                    v = &context->vertices[vertIndex];
                    x1 = v->x;
                    y1 = v->y;
                    z1 = v->z;
                    v = &context->vertices[nextVertIndex];
                    x2 = v->x;
                    y2 = v->y;
                    z2 = v->z;
                    x3 = x5 * 10.0f + x1;
                    y3 = y5 * 10.0f + y1;
                    z3 = z5 * 10.0f + z1;
                    x5 = (y2 - y1) * (z3 - z2) - (z2 - z1) * (y3 - y2);
                    y5 = (z2 - z1) * (x3 - x2) - (x2 - x1) * (z3 - z2);
                    z5 = (x2 - x1) * (y3 - y2) - (y2 - y1) * (x3 - x2);
                    mag = sqrtf(x5 * x5 + y5 * y5 + z5 * z5);
                    if (mag > 0.0f) {
                        x5 /= mag;
                        y5 /= mag;
                        z5 /= mag;
                    }
                    if (neighbor == 0xFFFE) {
                        group->points[i].unknown00 |= 1 << edge;
                    } else {
                        j = 0;
                        do {
                            if (scratch[neighbor].edgeNeighbor[j] == i) {
                                output->records[neighbor].edgeNeighbor[j] = counter | 0x8000;
                                scratch[neighbor].edgeNeighbor[j] = 0xFFFF;
                            }
                            j++;
                        } while (j < 3);
                        v = &context->vertices[oppVertIndex];
                        x3 = v->x;
                        y3 = v->y;
                        z3 = v->z;
                        mag = x3 * plane[0] + y3 * plane[1] + z3 * plane[2] + plane[3];
                        if (mag < 0.0f) {
                            group->points[i].unknown00 |= 1 << edge;
                        }
                    }
                    output->records[i].edgeNeighbor[edge] = counter;
                    scratch[i].edgeNeighbor[edge] = 0xFFFF;
                    output->planes[counter << 2] = x5;
                    output->planes[(counter << 2) + 1] = y5;
                    output->planes[(counter << 2) + 2] = z5;
                    output->planes[(counter << 2) + 3] = -(x1 * x5 + y1 * y5 + z1 * z5);
                    counter++;
                }
            }
        }
    }
    o19FreeReloc(scratch);
    return counter;
}
