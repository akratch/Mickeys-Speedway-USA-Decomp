#include "ultra64.h"

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

extern s32 D_o35_skip_collision_edges;
extern void *call_o0_0_2AE30(s32 size, s32 tag);
extern void call_o0_0_2B318(void *value);
extern f32 sqrtf(f32 value);

/*
 * Delta 0 at 130 masked words (lane B3-rl35): rebuilt on DKR's
 * track_init_collision shape. Size came from span fields read by subscript
 * (SR owns the span offset), plane stores indexed by counter << 2 with the
 * increment after them, s32 neighbour, and the opposite-vertex test through
 * a plane pointer taken before the edge maths. pad0, span and spanOffset
 * are unreferenced frame holders (L99). Left: IV creation order in the edge
 * loop (scratch + i * 8 before i * 8) rotates the t6/t9 ring, and s5-s7
 * colour order.
 *
 * PROVENANCE: adapted from Diddy Kong Racing,
 * src/object_models.c (model_init_collision).
 */
#ifdef NON_MATCHING
s32 func_overlay_035_F0000B40_1882820(O35CollisionSegment *s) {
    s32 pad0;
    O35CollisionRecord *scratch;
    O35CollisionRecord *scratchRecord;
    O35CollisionRecord *record;
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

    scratch = call_o0_0_2AE30(
        s->triangleCount * (s32)sizeof(O35CollisionRecord), 0x91);
    record = s->records;
    scratchRecord = scratch;
    copyIndex = 0;
    if (s->triangleCount * 4 > 0) {
        do {
            copyIndex++;
            scratchRecord = (O35CollisionRecord *)((u8 *)scratchRecord + 2);
            ((s16 *)scratchRecord)[-1] = *(s16 *)record;
            record = (O35CollisionRecord *)((u8 *)record + 2);
        } while (copyIndex < s->triangleCount * 4);
    }

    counter = 0;
    for (spanIndex = 0; spanIndex < s->spanCount; spanIndex++) {
        triStart = s->spans[spanIndex].triangleStart;
        vertexBase = s->spans[spanIndex].vertexBase;
        triEnd = s->spans[spanIndex + 1].triangleStart;
        if (s->spans[spanIndex].flags & 0x1080) {
            triStart = triEnd;
        }
        for (i = triStart; i < triEnd; i++) {
            v = &s->vertices[s->triangles[i].selectors[0] + vertexBase];
            x1 = v->x;
            y1 = v->y;
            z1 = v->z;
            v = &s->vertices[s->triangles[i].selectors[1] + vertexBase];
            x2 = v->x;
            y2 = v->y;
            z2 = v->z;
            v = &s->vertices[s->triangles[i].selectors[2] + vertexBase];
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
            s->records[i].plane = counter;
            s->planes[counter << 2] = nx;
            s->planes[(counter << 2) + 1] = ny;
            s->planes[(counter << 2) + 2] = nz;
            s->planes[(counter << 2) + 3] = -(x1 * nx + y1 * ny + z1 * nz);
            counter++;
        }
    }

    if (D_o35_skip_collision_edges != 0) {
        call_o0_0_2B318(scratch);
        return counter;
    }

    for (spanIndex = 0; spanIndex < s->spanCount; spanIndex++) {
        triStart = s->spans[spanIndex].triangleStart;
        vertexBase = s->spans[spanIndex].vertexBase;
        triEnd = s->spans[spanIndex + 1].triangleStart;
        if (s->spans[spanIndex].flags & 0x1080) {
            triStart = triEnd;
        }
        for (i = triStart; i < triEnd; i++) {
            idx = s->records[i].plane;
            nx = s->planes[4 * idx + 0];
            ny = s->planes[4 * idx + 1];
            nz = s->planes[4 * idx + 2];
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
                if (neighbor != 0xFFFF) {
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
                    mag = sqrtf(x5 * x5 + y5 * y5 + z5 * z5);
                    if (mag > 0.0f) {
                        x5 /= mag;
                        y5 /= mag;
                        z5 /= mag;
                    }
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
                        if (plane[3] + (x3 * plane[0] + y3 * plane[1] + z3 * plane[2]) <
                            0.0f) {
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
    }
    call_o0_0_2B318(scratch);
    return counter;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o035/func_overlay_035_F0000B40_1882820/func_overlay_035_F0000B40_1882820.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_035_F0000B40_1882820:start
 * symbol: func_overlay_035_F0000B40_1882820
 * score: 130/528 words
 * frame: 0x130
 * relocations: 7
 * first-mismatch: +0xA4
 * summary: Delta 0 at 130 on the DKR collision shape. Left: edge-loop IV order (i*8 before scratch+i*8) rotates t6/t9; s5-s7 colour order.
 * PLATEAU-HANDOFF:func_overlay_035_F0000B40_1882820:end
 */
