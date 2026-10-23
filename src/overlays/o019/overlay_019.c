#include "overlays/overlay_019.h"

/*
 * Overlay 19: one translation unit in ROM order (ADR 0006). overlay19Dispatch
 * is the module's only ORT export; every other function is static, which is
 * what makes their shipped calls JUMP records.
 */

static void overlay19BuildOutput(
    O19Context *context, O19BuildGroup *group, O19Output **outputSlot);
static s32 overlay19BuildPlanes(
    O19Context *context, O19Group *group, O19Output *output);
static void overlay19BuildAdjacency(
    O19Context *context, O19Group *group, O19Output *output);
static s32 overlay19FindAdjacent(
    O19Context *context, O19Group *group, s32 selfItem,
    s32 queryStartIndex, s32 queryEndIndex);
static s32 overlay19ClassifyEdge(
    O19Vertex *vertices, s32 queryStartIndex, s32 queryEndIndex,
    s32 candidateStartIndex, s32 candidateEndIndex);
static void overlay19BuildSpatialMasks(
    O19Context *context, O19Group *group, O19Output *output);

/* Pinned DKR v77/v80 and JFG donor scans classify overlay 19 as none. */
void overlay19Dispatch(Overlay19Object *object) {
    Overlay19Header *header;
    s32 selected;
    s32 index;
    s32 offset;
    void *item;
    void *data;

    header = object->header;
    selected = header->selected;
    if (selected >= 0) {
        item = object->items[selected];
        data = *(void **)item;
        overlay19BuildOutput(item, data, (O19Output **)((u8 *)data + 0x58));
    } else {
        index = 0;
        offset = 0;
        if (header->count > 0) {
            do {
                item = *(void **)((u8 *)object->items + offset);
                data = *(void **)item;
                overlay19BuildOutput(item, data, (O19Output **)((u8 *)data + 0x58));
                index++;
                offset += 4;
            } while (index < object->header->count);
        }
    }
}

static void overlay19BuildOutput(
    O19Context *context, O19BuildGroup *group, O19Output **outputSlot) {
    O19Output *output;
    u8 *cursor;
    void *state;
    s32 used;

    if (group->gate4E != 0 || *outputSlot != 0) {
        return;
    }

    output = mmAlloc2(0x4000, 0x8A);
    if (output == 0) {
        return;
    }

    *outputSlot = output;
    cursor = (u8 *)output + sizeof(O19Output);
    output->records = (O19AdjacencyRecord *)cursor;

    overlay19BuildAdjacency(context, (O19Group *)group, *outputSlot);
    cursor += group->itemCount * sizeof(O19AdjacencyRecord);
    (*outputSlot)->masks = (u32 *)cursor;

    overlay19BuildSpatialMasks(context, (O19Group *)group, *outputSlot);
    cursor += group->itemCount * sizeof(u32);
    (*outputSlot)->planes = (f32 *)cursor;

    cursor += overlay19BuildPlanes(
        context, (O19Group *)group, *outputSlot) * 0x10;
    used = cursor - (u8 *)output;
    if (used >= 0x4001) {
        mmFree(*outputSlot);
        *outputSlot = 0;
        return;
    }

    state = mmGetDelay();
    mmSetDelay(0);
    mmFree(output);
    mmAllocAtAddr(used, output, 0x8A);
    mmSetDelay(state);
}

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
static s32 overlay19BuildPlanes(O19Context *context, O19Group *group, O19Output *output) {
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

    scratch = mmAlloc2(group->itemCount * (s32)sizeof(O19AdjacencyRecord), 0x8A);
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
    mmFree(scratch);
    return counter;
}

/* Tier A: the call-site fifth-argument assignment preserves IDO's selector
 * emission order and the target's 492-byte body. The assignment only names
 * the argument value; its local is not observed by any other argument.
 * Keep the two independent outer-loop updates on one physical line. */
static void overlay19BuildAdjacency(
    O19Context *context,
    O19Group *group,
    O19Output *output) {
    struct A30Frame {
        s32 outputOffset;
        s32 spanOffset;
        s32 suppressed;
        u8 pad0C[0x18];
        s32 itemEnd;
        u8 pad28[0x0C];
        s32 spanIndex;
    } frame;
    s32 itemIndex;
    s32 edgeIndex;
    s32 nextEdgeIndex;
    s32 edgeOffset;
    s32 adjacentItem;
    s32 edgeLimit;
    u16 invalid;
    s16 vertexBase;
    s16 spanCount;
    s16 itemStart;
    O19Span *span;

    frame.spanIndex = 0;
    spanCount = group->spanCount;
    if (spanCount > 0) {
        frame.spanOffset = 0;
        edgeLimit = 6;
        invalid = 0xFFFF;
        do {
            span = (O19Span *)((u8 *)group->spans + frame.spanOffset);
            frame.itemEnd = (span + 1)->itemStart +
                (itemStart = span->itemStart,
                 vertexBase = span->vertexBase,
                 0);
            itemIndex = itemStart;
            adjacentItem = span->flags;
            if (itemStart < frame.itemEnd) {
                adjacentItem &= 0x1080;
                frame.suppressed = adjacentItem; frame.outputOffset = itemIndex << 3;
                do {
                    if (frame.suppressed != 0) {
                        *(u16 *)((u8 *)output->records + frame.outputOffset) = invalid;
                        edgeIndex = 0;
                        do {
                            *(u16 *)&output->records[itemIndex]
                                .edgeNeighbor[edgeIndex] = invalid;
                            edgeIndex++;
                        } while (edgeIndex < 3);
                    } else {
                        *(s16 *)((u8 *)output->records + frame.outputOffset) =
                            (s16)itemIndex;
                        edgeIndex = 0;
                        do {
                            nextEdgeIndex = edgeIndex + 1;
                            if (nextEdgeIndex >= 3) {
                                nextEdgeIndex = 0;
                            }
                            adjacentItem = overlay19FindAdjacent(
                                context,
                                group,
                                itemIndex,
                                group->points[itemIndex].selectors[edgeIndex] +
                                    vertexBase,
                                (edgeOffset = group->points[itemIndex].selectors[nextEdgeIndex] + vertexBase));
                            if (adjacentItem == -1) {
                                *(u16 *)&output->records[itemIndex]
                                    .edgeNeighbor[edgeIndex] = 0xFFFE;
                            } else {
                                output->records[itemIndex]
                                    .edgeNeighbor[edgeIndex] =
                                    (s16)adjacentItem;
                            }
                            edgeIndex++;
                        } while (edgeIndex != 3);
                    }
                    itemIndex++;
                    frame.outputOffset += 8;
                } while (itemIndex < frame.itemEnd);
            }
            frame.spanIndex++; frame.spanOffset += sizeof(O19Span);
            spanCount = group->spanCount;
        } while (frame.spanIndex < spanCount);
    }
}

/*
 * Pinned DKR v77/v80 and JFG donor scans classify overlay 19 as none.
 * IDO appends one alignment word after this 0x15C-byte function; the build
 * rule trims that non-owned word because the following D78 body starts there.
 */
static s32 overlay19FindAdjacent(
    O19Context *context,
    O19Group *group,
    s32 selfItem,
    s32 queryStartIndex,
    s32 queryEndIndex) {
    s32 spanIndex;
    u8 unusedStackShape[0x24];
    s32 spanOffset;
    s32 candidateStart;
    s32 candidateItem;
    s32 itemEnd;
    s32 edgeIndex;
    s32 nextEdgeIndex;
    s32 classification;
    s32 candidateStartIndex;
    s32 candidateEndIndex;
    s16 vertexBase;
    O19Span *span;

    spanIndex = 0;
    if (group->spanCount > 0) {
        spanOffset = 0;
        do {
            span = (O19Span *)((u8 *)group->spans + spanOffset);
            candidateStart = span->itemStart;
            vertexBase = span->vertexBase;
            itemEnd = (span + 1)->itemStart;
            if ((span->flags & 0x1080) != 0) candidateStart = itemEnd;
            if (candidateStart < itemEnd) {
                candidateItem = candidateStart;
                do {
                    if (candidateItem != selfItem) {
                        edgeIndex = 0;
                        do {
                            nextEdgeIndex = edgeIndex + 1;
                            if (nextEdgeIndex >= 3) nextEdgeIndex = 0;
                            candidateStartIndex = group->points[candidateItem].selectors[edgeIndex] + vertexBase;
                            candidateEndIndex = group->points[candidateItem].selectors[nextEdgeIndex] + vertexBase;
                            classification = overlay19ClassifyEdge(
                                context->vertices, queryStartIndex, queryEndIndex,
                                candidateStartIndex, candidateEndIndex);
                            if (classification != 0) return candidateItem;
                            edgeIndex++;
                        } while (edgeIndex < 3);
                    }
                    candidateItem++;
                } while (candidateItem != itemEnd);
            }
            spanOffset += (spanIndex++, sizeof(O19Span));
        } while (spanIndex < group->spanCount);
    }
    return -1;
}

/*
 * Classify an edge against a candidate edge, accepting either orientation.
 * PROVENANCE: Mickey-derived. The exact-donor ledger is negative. A retained
 * structural scan found JFG assembly-only
 * func_overlay_30_000012F0_1F4A3C0 at 0.443 and DKR public
 * src/object_models.c:func_80060C58, a five-argument edge classifier, at 0.227;
 * neither is an exact donor and no external C is adapted here.
 * The sole inbound is overlay19FindAdjacent+0xD8; this function is unexported.
 *
 * Matched 2026-09-10 by two edits, neither of which works alone:
 *
 * 1. The reversed-orientation block compares against `queryStartX` rather than
 *    re-reading `candidateEnd->y`/`->z`. The direct read is a separate common
 *    subexpression, and its web is barred from `queryStartX`'s register
 *    because the staged variable stays live into the trailing block; comparing
 *    through the staged name lets the load land in that register directly, the
 *    way the forward block already did.
 * 2. `candidateStart` is defined above the early-exit test rather than below
 *    it. Web identifiers follow first-definition order and the second
 *    allocator phase colours in that order, so hoisting the definition puts
 *    this web ahead of the stack-passed fifth argument's and it takes the
 *    first free colour. Written below the test, the argument is coloured
 *    first and the two swap registers across sixteen words.
 */
static s32 overlay19ClassifyEdge(
    O19Vertex *vertices,
    s32 queryStartIndex,
    s32 queryEndIndex,
    s32 candidateStartIndex,
    s32 candidateEndIndex) {
    O19Vertex *queryStart;
    O19Vertex *queryEnd;
    O19Vertex *candidateStart;
    O19Vertex *candidateEnd;
    s32 queryStartX;
    s32 queryEndX;
    s32 candidateStartX;
    s32 candidateEndX;
    s32 candidateStartXMinusOne;

    candidateStart = &vertices[candidateStartIndex];
    if (((queryStartIndex == candidateStartIndex) &&
         (queryEndIndex == candidateEndIndex)) ||
        ((queryStartIndex == candidateEndIndex) &&
         (queryEndIndex == candidateStartIndex))) {
        return 1;
    }

    queryStart = &vertices[queryStartIndex];
    candidateStartX = candidateStart->x;
    candidateStartXMinusOne = candidateStartX - 1;
    queryStartX = queryStart->x;

    if (!(candidateStartXMinusOne < queryStartX) ||
        (candidateStartX < queryStartX)) {
        goto check_reversed_coordinates;
    }
    if (candidateStart->y >= (queryStart->y + 1)) {
        goto check_reversed_coordinates;
    }
    if (queryStart->y > candidateStart->y) {
        goto check_reversed_coordinates;
    }
    if (candidateStart->z >= (queryStart->z + 1)) {
        goto check_reversed_coordinates;
    }
    if (queryStart->z > candidateStart->z) {
        goto check_reversed_coordinates;
    }

    queryEnd = &vertices[queryEndIndex];
    candidateEnd = &vertices[candidateEndIndex];
    candidateEndX = candidateEnd->x;
    queryEndX = queryEnd->x;
    if (queryEndX < candidateEndX) {
        goto no_match;
    }
    if (candidateEndX < queryEndX) {
        goto no_match;
    }
    queryStartX = candidateEnd->y;
    if (queryEnd->y < queryStartX) {
        goto no_match;
    }
    if (queryStartX < queryEnd->y) {
        goto no_match;
    }
    queryStartX = candidateEnd->z;
    if (queryEnd->z < queryStartX) {
        goto no_match;
    }
    if (queryStartX < queryEnd->z) {
        goto no_match;
    }
    return 2;

check_reversed_coordinates:
    candidateEnd = &vertices[candidateEndIndex];
    candidateEndX = candidateEnd->x;
    if (queryStartX < candidateEndX) {
        goto no_match;
    }
    if (candidateEndX < queryStartX) {
        goto no_match;
    }
    queryStartX = candidateEnd->y;
    if (queryStart->y < queryStartX) {
        goto no_match;
    }
    if (queryStartX < queryStart->y) {
        goto no_match;
    }
    queryStartX = candidateEnd->z;
    if (queryStart->z < queryStartX) {
        goto no_match;
    }
    if (queryStartX < queryStart->z) {
        goto no_match;
    }

    queryEnd = &vertices[queryEndIndex];
    queryEndX = queryEnd->x;
    if (!(candidateStartXMinusOne < queryEndX) ||
        (candidateStartX < queryEndX)) {
        goto no_match;
    }
    queryStartX = candidateStart->y;
    if (queryEnd->y < candidateStart->y) {
        goto no_match;
    }
    if (queryStartX < queryEnd->y) {
        goto no_match;
    }
    queryStartX = candidateStart->z;
    if (queryEnd->z < candidateStart->z) {
        goto no_match;
    }
    if (queryStartX < queryEnd->z) {
        goto no_match;
    }
    return 2;

no_match:
    return 0;
}

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
static void overlay19BuildSpatialMasks(O19Context *context, O19Group *group, O19Output *output) {
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
