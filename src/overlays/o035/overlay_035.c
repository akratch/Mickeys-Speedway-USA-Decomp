#include "overlays/overlay_035.h"

s32 *gOverlay35ModelTable;
s32 gOverlay35SkipCollisionEdges;

/*
 * Overlay 35: one translation unit in ROM order (ADR 0006). Only the
 * initializer (+0x0) is an ORT export; the model loader, grid-mask builder,
 * collision-plane builder and height selector are static, which is what
 * makes the shipped calls to them JUMP records.
 */

static void func_overlay_035_F00001E0_1881EC0(s32 modelId);
static void func_overlay_035_F0000770_1882450(O35GridSource *source,
                                             O35GridBounds *bounds);
static s32 func_overlay_035_F0000B40_1882820(O35CollisionSegment *s);
static void overlay35SelectHeight(Overlay35State *state);

void func_overlay_035_F0000000_1881CE0(s32 modelId, s32 value1, s32 value2,
                                        s32 value3, s32 value4, s32 value5) {
    O35Level *system;

    D_800C9548 = mmAlloc(0x800, 0x8F);
    D_800792EC = levelGetLevel();
    gOverlay35SkipCollisionEdges = 0;
    if ((D_800792EC->flagF6 == 0) &&
        ((D_800792EC->mode83 == 1) || (D_800792EC->mode83 == 2))) {
        gOverlay35SkipCollisionEdges = 1;
    }

    func_overlay_035_F00001E0_1881EC0(modelId);
    D_800C99BC = 0;
    camSetMode(mainGetNumberOfCameras() - 1);
    func_80004C28(value4, 0);
    func_80004C28(value3, 1);
    animseqLoadLevelData(value5);
    func_80004FE0(value2);

    D_800C9544 = 0;
    shadowInitBuffers(D_800792E8, D_800C99BC, 0x1F4, 0xC8, 0x64);
    shadowGenerate(2, 0);
    shadowGenerate(1, 0);
    shadowChangeBuffer();
    shadowGenerate(2, 0);
    shadowGenerate(1, 0);
    shadowChangeBuffer();

    if (D_800792EC->flag82 == 0) {
        modelSetModelFlags(0);
    }
    func_8000C540(value1);
    if (D_800792EC->flag82 == 0) {
        modelSetModelFlags(8);
    }

    D_800C9560 = 0;
    D_800C9568 = 0x100000;
    if (D_800792EC->handle92 != -1) {
        D_800792F0 = texLoadTexture(D_800792EC->handle92);
        D_800792F4 = 0; }
    D_80079274 = 0;
    if (D_800792EC->flagF6 != 0) {
        overlay98CollectUniqueY(D_800792E8);
    }
}

/* Matched by spelling every model access as D_800792E8 (DKR style) so
 * uopt CSEs the loop-condition reloads, testing the heap through the current
 * model, walking gOverlay35ModelTable directly (its address web then outranks
 * 0x9F000 for s3), post-decrement copy loops (the target's dead count copy),
 * and reading the walkers back through the stored colorData fields.
 * The unreferenced pad keeps mdl's home at sp+0x38 (L99).
 * PROVENANCE: adapted from Diddy Kong Racing, src/tracks.c (generate_track). */
static void func_overlay_035_F00001E0_1881EC0(s32 modelId) {
    void *unusedPad;
    s32 mdl;
    s32 i;
    s32 j;
    s32 k;
    s32 byteCount;
    s32 temp;
    O35Segment *segment;
    u8 *cursor;

    D_800C9D38 = D_800792EC->allocationCount;
    D_800C9D2C = mmAlloc(D_800C9D38 * 4, 0x91);
    D_800C9D30 = mmAlloc(D_800C9D38 * 2, 0x91);
    D_800C9D34 = mmAlloc(D_800C9D38 * 2, 0x91);
    D_800C9D20 = mmAlloc(
        D_800792EC->segmentAllocationCount * 0x2C, 0x91);
    D_800C9D3C = 0;
    D_800C95A8 = mmAlloc(0x9F000, 0x91);
    D_800792E8 = D_800C95A8;
    if (D_800792E8 == 0) {
        func_8002A244();
    }
    mainPreNMI();

    gOverlay35ModelTable = (s32 *) piRomLoad(0x24);
    for (i = 0; gOverlay35ModelTable[i] != -1; i++) {
    }
    i--;
    if (modelId >= i) {
        modelId = 0;
    }

    mdl = gOverlay35ModelTable[modelId];
    byteCount = gOverlay35ModelTable[modelId + 1] - mdl;
    temp = (s32)D_800792E8;
    temp += 0x9F000 - byteCount;
    temp -= temp % 16;

    mainPreNMI();
    piRomLoadSection(0x25, (void *) temp, mdl, byteCount);
    func_8004D7E0((void *)temp, D_800792E8);
    mmFree(gOverlay35ModelTable);
    mainPreNMI();

    mdl = (s32)D_800792E8;
    D_800792E8->textures =
        (O35TextureInfo *)((s32)D_800792E8->textures + mdl);
    D_800792E8->segments =
        (O35Segment *)((s32)D_800792E8->segments + mdl);
    D_800792E8->bounds =
        (O35Bounds *)((s32)D_800792E8->bounds + mdl);
    D_800792E8->unkC = (void *)((s32)D_800792E8->unkC + mdl);
    D_800792E8->unk10 =
        (void *)((s32)D_800792E8->unk10 + mdl);
    D_800792E8->unk14 =
        (void *)((s32)D_800792E8->unk14 + mdl);

    for (k = 0; k < D_800792E8->segmentCount; k++) {
        D_800792E8->segments[k].vertices =
            (u8 *)((s32)D_800792E8->segments[k].vertices + mdl);
        D_800792E8->segments[k].triangles =
            (void *)((s32)D_800792E8->segments[k].triangles + mdl);
        D_800792E8->segments[k].groups =
            (void *)((s32)D_800792E8->segments[k].groups + mdl);
        D_800792E8->segments[k].unk18 =
            (void *)((s32)D_800792E8->segments[k].unk18 + mdl);
    }

    cursor = (u8 *)D_800792E8 + D_800792E8->modelSize;
    for (k = 0; k < D_800792E8->segmentCount; k++) {
        cursor = align4(cursor);
        D_800792E8->segments[k].masks = (u32 *)cursor;
        cursor += D_800792E8->segments[k].dataOffset * 4;
        D_800792E8->segments[k].collisionFacets = cursor;
        cursor += D_800792E8->segments[k].dataOffset;
        func_overlay_035_F0000770_1882450(
            (O35GridSource *)&D_800792E8->segments[k],
            (O35GridBounds *)&D_800792E8->bounds[k]);
        cursor = align16(cursor);
        D_800792E8->segments[k].collisionPlanes = (f32 *)cursor;
        cursor += func_overlay_035_F0000B40_1882820(
                      (O35CollisionSegment *)&D_800792E8->segments[k]) *
                  0x10;
        D_800792E8->segments[k].selectedValue = 0;
        overlay35SelectHeight(
            (Overlay35State *)&D_800792E8->segments[k]);
    }

    cursor = align16(cursor);
    byteCount = cursor - (u8 *)D_800792E8;
    mainPreNMI();
    temp = mmGetDelay();
    mmSetDelay(0);
    mmFree(D_800C95A8);
    mmAllocAtAddr(byteCount, D_800C95A8, 0x91);
    mmSetDelay(temp);
    mainPreNMI();

    for (k = 0; k < D_800792E8->textureCount; k++) {
        D_800792E8->textures[k].texture = texLoadTexture(
            (s32)D_800792E8->textures[k].texture | 0x8000);
    }
    mainPreNMI();

    segment = D_800792E8->segments;
    for (k = 0; k < D_800792E8->segmentCount; k++) {
        O35ColorData *colorData;
        u8 *colors;
        s16 *flags;

        j = segment->count20;
        byteCount = ((j + 0xF) >> 4) * 2;
        colorData = mmAlloc(byteCount + 8 + j * 3, 0x91);
        if (colorData != 0) {
            s32 remaining;
            u8 *source;

            segment->colorData = colorData;
            colorData->flags = (s16 *)((u8 *)colorData + 8);
            colorData->colors = (u8 *)colorData + byteCount + 8;
            remaining = segment->count20;
            flags = colorData->flags;
            colors = colorData->colors;
            source = segment->vertices;
            while (remaining--) {
                *colors++ = source[6];
                *colors++ = source[7];
                *colors++ = source[8];
                source += 0xA;
            }
            remaining = byteCount >> 1;
            while (remaining--) {
                *flags++ = 0;
            }
            segment->flag2E = 0;
        }
        segment++;
    }
    mainPreNMI();
    D_80079314 = 0;
    trackLightAllocate(D_800792EC->finalArg0,
                   D_800792EC->finalArg1);
}

static void func_overlay_035_F0000770_1882450(O35GridSource *source,
                                        O35GridBounds *bounds) {
    O35Vertex *vertices;
    s32 vertex;
    s32 mask;
    s32 end;
    s16 startVertex;
    s16 baseIndex;
    s32 groupIndex;
    O35Vertex *v;
    u8 *index;
    s16 maxX;
    s16 maxY;
    s16 maxZ;
    s16 minX;
    s16 minY;
    s16 minZ;
    s32 i;
    s32 bit;
    s32 flags;
    u8 zMask;
    s16 start;
    s16 step;
    s16 value;
    s16 origin;
    s16 x;
    s16 y;
    s16 z;

    groupIndex = 0;
    if (source->groupCount > 0) {
        do {
            startVertex = *(s16 *)(source->groups + groupIndex * 0x10 + 8);
            baseIndex = *(s16 *)(source->groups + groupIndex * 0x10 + 6);
            end = *(s16 *)(source->groups + groupIndex * 0x10 + 0x18);
            flags = *(s32 *)(source->groups + groupIndex * 0x10 + 0xC);
            vertex = startVertex;
            if (vertex < end) {
                do {
                    if (((flags & 0x1080) != 0) && ((flags & 0x08010000) == 0)) {
                        *(u32 *)((u8 *)source->masks + vertex * 4) = 0;
                        source->zMasks[vertex] = 0;
                    } else {
                        maxX = -0x7FBC;
                        maxY = -0x7FBC;
                        maxZ = -0x7FBC;
                        minX = 0x7FBC;
                        minY = 0x7FBC;
                        minZ = 0x7FBC;
                        index = source->indices + vertex * 0x10;
                        i = 0;
                        vertices = source->vertices;
                        for (; i < 3; i++) {
                            v = &vertices[index[i + 1] + baseIndex];
                            x = v->x;
                            y = v->y;
                            z = v->z;
                            if (maxX < x) maxX = x;
                            if (x < minX) minX = x;
                            if (maxY < y) maxY = y;
                            if (y < minY) minY = y;
                            if (maxZ < z) maxZ = z;
                            if (z < minZ) minZ = z;
                        }

                        bit = 1;
                        mask = 0;
                        origin = bounds->x0;
                        step = ((bounds->x1 - origin) >> 4) + 1;
                        value = step + origin;
                        start = origin;
                        for (x = 0; x < 16; x++) {
                            if ((value >= minX) && (maxX >= start)) {
                                mask |= bit;
                            }
                            value += step; start += step;
                            bit *= 2;
                        }

                        origin = bounds->y0;
                        step = ((bounds->z1 - origin) >> 4) + 1;
                        value = step + origin;
                        start = origin;
                        for (x = 0; x < 16; x++) {
                            if ((value >= minZ) && (maxZ >= start)) {
                                mask |= bit;
                            }
                            value += step; start += step;
                            bit *= 2;
                        }
                        *(u32 *)((u8 *)source->masks + vertex * 4) = mask;

                        bit = 1;
                        zMask = 0;
                        step = ((bounds->y1 - bounds->z0) >> 3) + 1;
                        origin = bounds->z0;
                        value = step + origin;
                        start = origin;
                        for (x = 0; x < 8; x++) {
                            if ((value >= minY) && (maxY >= start)) {
                                zMask |= bit;
                            }
                            value += step; start += step;
                            bit *= 2;
                        }
                        source->zMasks[vertex] = zMask;
                    }
                    vertex++;
                } while (vertex < end);
            }
            groupIndex++;
        } while (groupIndex < source->groupCount);
    }
}

/*
 * Matched (lane j-o035, 2026-10-02) on DKR's track_init_collision shape.
 * The second pass reads its plane index as `idx = plane * 4` in one
 * expression. The span reads, the normalisation in both passes and the
 * first pass's second and third vertex reads go through do-while(0)
 * macros; their region blocks set the saved-register order of the counter,
 * span index and offset, edge index, opposite vertex and edge-offset webs
 * (measured, the macro split itself is not recovered from anything).
 *
 * PROVENANCE: adapted from Diddy Kong Racing, src/tracks.c
 * (track_init_collision) and src/object_models.c (model_init_collision).
 */
#define O35_SPAN_RANGE(s, index, start, base, end) \
    do { \
        start = (s)->spans[index].triangleStart; \
        base = (s)->spans[index].vertexBase; \
        end = (s)->spans[(index) + 1].triangleStart; \
    } while (0)

#define O35_READ_VERTEX(v, s, index, outX, outY, outZ) \
    do { \
        v = &(s)->vertices[index]; \
        outX = (v)->x; \
        outY = (v)->y; \
        outZ = (v)->z; \
    } while (0)

#define O35_NORMALIZE(x, y, z, mag) \
    do { \
        mag = sqrtf((x) * (x) + (y) * (y) + (z) * (z)); \
        if (mag > 0.0f) { \
            x /= mag; \
            y /= mag; \
            z /= mag; \
        } \
    } while (0)

static s32 func_overlay_035_F0000B40_1882820(O35CollisionSegment *s) {
    s32 pad0;
    O35CollisionRecord *scratch;
    s16 *scratchRecord;
    s16 *record;
    s32 copyIndex;
    f32 x1, y1, z1;
    f32 x2, y2, z2;
    f32 x3, y3, z3;
    f32 nx, ny, nz;
    f32 x5, y5, z5;
    f32 mag;
    s32 triStart;
    O35CollisionVertex *v;
    s32 i;
    O35CollisionSpan *span;
    s32 triEnd;
    s32 vertexBase;
    s32 spanIndex;
    s32 j;
    s32 edge;
    s32 counter;
    s32 oppVertIndex;
    s32 idx;
    s32 next;
    s32 opp;
    s32 vertIndex;
    s32 nextVertIndex;
    s32 neighbor;
    s32 spanOffset;
    f32 *plane;

    scratch = mmAlloc(
        s->triangleCount * (s32)sizeof(O35CollisionRecord), 0x91);
    record = (s16 *)s->records;
    scratchRecord = (s16 *)scratch;
    for (copyIndex = 0; copyIndex < s->triangleCount * 4; copyIndex++) {
        *scratchRecord++ = *record++;
    }

    counter = 0;
    for (spanIndex = 0; spanIndex < s->spanCount; spanIndex++) {
        O35_SPAN_RANGE(s, spanIndex, triStart, vertexBase, triEnd);
        if (s->spans[spanIndex].flags & 0x1080) {
            triStart = triEnd;
        }
        for (i = triStart; i < triEnd; i++) {
            v = &s->vertices[s->triangles[i].selectors[0] + vertexBase];
            x1 = v->x;
            y1 = v->y;
            z1 = v->z;
            O35_READ_VERTEX(v, s, s->triangles[i].selectors[1] + vertexBase, x2, y2, z2);
            O35_READ_VERTEX(v, s, s->triangles[i].selectors[2] + vertexBase, x3, y3, z3);
            nx = (y2 - y1) * (z3 - z2) - (z2 - z1) * (y3 - y2);
            ny = (z2 - z1) * (x3 - x2) - (x2 - x1) * (z3 - z2);
            nz = (x2 - x1) * (y3 - y2) - (y2 - y1) * (x3 - x2);
            O35_NORMALIZE(nx, ny, nz, mag);
            s->records[i].plane = counter;
            s->planes[counter << 2] = nx;
            s->planes[(counter << 2) + 1] = ny;
            s->planes[(counter << 2) + 2] = nz;
            s->planes[(counter << 2) + 3] = -(x1 * nx + y1 * ny + z1 * nz);
            counter++;
        }
    }

    if (gOverlay35SkipCollisionEdges != 0) {
        mmFree(scratch);
        return counter;
    }

    for (spanIndex = 0; spanIndex < s->spanCount; spanIndex++) {
        O35_SPAN_RANGE(s, spanIndex, triStart, vertexBase, triEnd);
        if (s->spans[spanIndex].flags & 0x1080) {
            triStart = triEnd;
        }
        for (i = triStart; i < triEnd; i++) {
            idx = s->records[i].plane * 4;
            nx = s->planes[idx + 0];
            ny = s->planes[idx + 1];
            nz = s->planes[idx + 2];
            for (edge = 0; edge < 3; edge++) {
                next = edge + 1;
                if (next >= 3) {
                    next = 0;
                }
                opp = next + 1;
                if (opp >= 3) {
                    opp = 0;
                }
                vertIndex = s->triangles[i].selectors[edge] + vertexBase;
                nextVertIndex = s->triangles[i].selectors[next] + vertexBase;
                oppVertIndex = s->triangles[i].selectors[opp] + vertexBase;
                neighbor = scratch[i].edgeNeighbor[edge];
                if (neighbor == 0xFFFF) {
                    continue;
                }
                if (neighbor == 0xFFFE) {
                    idx = s->records[i].plane * 4;
                } else {
                    idx = s->records[neighbor].plane * 4;
                }
                plane = &s->planes[idx];
                x5 = s->planes[idx + 0] + nx;
                y5 = s->planes[idx + 1] + ny;
                z5 = s->planes[idx + 2] + nz;
                v = &s->vertices[vertIndex];
                x1 = v->x;
                y1 = v->y;
                z1 = v->z;
                v = &s->vertices[nextVertIndex];
                x2 = v->x;
                y2 = v->y;
                z2 = v->z;
                x3 = x5 * 5.0f + x1;
                y3 = y5 * 5.0f + y1;
                z3 = z5 * 5.0f + z1;
                x5 = (y2 - y1) * (z3 - z1) - (z2 - z1) * (y3 - y1);
                y5 = (z2 - z1) * (x3 - x1) - (x2 - x1) * (z3 - z1);
                z5 = (x2 - x1) * (y3 - y1) - (y2 - y1) * (x3 - x1);
                O35_NORMALIZE(x5, y5, z5, mag);
                if (neighbor == 0xFFFE) {
                    s->triangles[i].flags |= 1 << edge;
                } else {
                    for (j = 0; j < 3; j++) {
                        if (scratch[neighbor].edgeNeighbor[j] == i) {
                            s->records[neighbor].edgeNeighbor[j] =
                                counter | 0x8000;
                            scratch[neighbor].edgeNeighbor[j] = 0xFFFF;
                        }
                    }
                    v = &s->vertices[oppVertIndex];
                    x3 = v->x;
                    y3 = v->y;
                    z3 = v->z;
                    mag = x3 * plane[0] + y3 * plane[1] + z3 * plane[2] + plane[3];
                    if (mag < 0.0f) {
                        s->triangles[i].flags |= 1 << edge;
                    }
                }
                s->records[i].edgeNeighbor[edge] = counter;
                scratch[i].edgeNeighbor[edge] = 0xFFFF;
                s->planes[counter << 2] = x5;
                s->planes[(counter << 2) + 1] = y5;
                s->planes[(counter << 2) + 2] = z5;
                s->planes[(counter << 2) + 3] = -(x1 * x5 + y1 * y5 + z1 * z5);
                counter++;
            }
        }
    }
    mmFree(scratch);
    return counter;
}

/* DKR v77/v80 and JFG have no exact flagged-record height selector. */
static void overlay35SelectHeight(Overlay35State *state) {
    s32 index;
    Overlay35Record *record;

    index = 0;
    state->selectedValue = -10000;
    if (state->count > 0) {
        record = state->records;
        do {
            index++;
            if (record->flags & 0x10000) {
                state->selectedValue = state->values[record->valueIndex].value;
            }
            record++;
        } while (index < state->count);
    }
}
