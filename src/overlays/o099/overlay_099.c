#include "overlays/overlay_099.h"

/* ---- overlay99GetEntries (+0x0) ---- */

/* DKR v77/v80 and JFG checks found no exact donor for this base accessor. */
void *overlay99GetEntries(void) {
    return gOverlay99Entries;
}

/* DKR v77/v80 and JFG checks found only generic resource-array cleanup. */
void overlay99ReleaseEntries(void) {
    Overlay99Entry *entry = gOverlay99Entries;
    s32 remaining = 2;

    do {
        if (entry->object2C != NULL) {
            func_80006EA0(entry->object2C);
            entry->object2C = NULL;
        }
        entry++;
    } while (remaining--);
}

/* Declaring the address-taken descriptor after the entry cursor gives it the
 * retail sp+0x54 home. The resulting 110 words and all seven linked runtime
 * relocation roles are exact. */
void overlay99InitializeEntries(s32 count, Overlay99Entry *source,
                                       f32 x, f32 y) {
    u16 reservedStack[3];
    Overlay99Entry *entry;
    Overlay99SpawnDescriptor descriptor;
    s32 created;
    s32 i;

    func_80037414(4, x, y, 0, 0, 0, 0);
    i = 0;
    created = 0;
    entry = gOverlay99Storage.entries;
    if (count > 0) {
        do {
            entry->value00 = source->value00;
            entry->value04 = source->value04;
            entry->value08 = source->value08;
            entry->value0C = source->value0C;
            entry->value10 = source->value10;
            entry->value14 = source->value14;
            entry->value18 = source->value18;
            entry->value1C = source->value1C;
            entry->value20 = source->value20;
            entry->value24 = source->value24;
            entry->kind28 = source->kind28;
            entry->mode2A = source->mode2A;
            entry->object2C = 0;
            if (entry->kind28 != -1) {
                descriptor.kind0 = entry->kind28;
                descriptor.x4 = (s32)source->value00;
                descriptor.y6 = (s32)source->value04;
                descriptor.height8 = 5;
                descriptor.modeA = source->spawnMode2B;
                descriptor.flagsB = 0x40;
                descriptor.zeroC = 0;
                descriptor.zeroD = 0;
                entry->object2C = func_8000590C(&descriptor, 1);
                if (entry->object2C != 0) {
                    entry->object2C->owner3C = 0;
                    entry->object2C->flags6 |= 0x400;
                }
                created++;
            }
            i++;
            source++;
            entry++;
        } while ((i < count) && (i != 3));
    }
    gOverlay99Storage.count = i;
    if (created > 0) {
        viFrameRateReset();
    }
}

/* Pinned DKR v77/v80 and JFG object/source checks found no donor. */
f32 overlay99ProjectVector(f32 x, f32 y, f32 z, f32 dx, f32 dy) {
    f32 result;

    result = (x * x) + (y * y);
    if (result > 0.0f) {
        result = ((x * dx) + (y * dy) + z) / sqrtf(result);
    }
    return result;
}

/* 65536 / 360 as the shipped pool spells it, four decimals. */
#define G_ANGLE_UNITS_PER_DEGREE 182.0444f

/* Matched 2026-10-02, 190 words to 0, written from the listing. The
 * parameter t is reused for the side distance and one local carries both the
 * along fraction and the edge wave: those two reuses are what order the six
 * saved float registers. The plane terms are plain locals (the middle ones
 * stay in memory on their own, no volatile), the segment ends are re-read
 * from the record, the grid pointer is loaded last, and the final product
 * goes through a local before the conversion. -Wab,-r4300_mul supplies the
 * three multiply-hazard pads. */
void overlay99ApplySegment(Overlay99Influence *influence, f32 t) {
    s32 i;
    s32 j;
    Overlay99Vertex *point;
    s32 phase;
    f32 x;
    f32 z;
    f32 distance;
    f32 along;
    f32 wave;
    f32 amount;
    s32 pad;
    f32 invLength;
    f32 invAngle;
    f32 width;
    f32 edge;
    f32 planeD;
    f32 normalZ;
    f32 deltaX;
    f32 sideD;
    f32 normalX;

    x = influence->x0 + ((influence->x1 - influence->x0) * t);
    z = influence->z0 + ((influence->z1 - influence->z0) * t);
    normalZ = influence->z0 - z;
    deltaX = x - influence->x0;
    normalX = -deltaX;
    sideD = -((influence->x0 * normalZ) + (influence->z0 * deltaX));
    planeD = -((x * normalX) + (z * normalZ));
    invLength = 1.0f / influence->longitudinalScale;
    phase = (s32)(influence->angleDegrees * G_ANGLE_UNITS_PER_DEGREE);
    invAngle = 1.0f / influence->angleScale;
    point = D_8007BE88[D_8007BE84];
    if (point != 0) {
        for (j = 0; j < gOverlay99GridHeight; j++) {
            for (i = 0; i < gOverlay99GridWidth; i++) {
                x = (f32)(i - (gOverlay99WidthMinusOne >> 1)) *
                    (f32)gOverlay99Arg4;
                z = (f32)((gOverlay99HeightMinusOne >> 1) - j) *
                    (f32)gOverlay99Arg5;
                distance = overlay99ProjectVector(normalX, normalZ, planeD,
                                                  x, z);
                if (distance > 0.0f) {
                    along = distance * invLength;
                    if ((along > 0.0f) && (along < 1.0f)) {
                        wave = func_8002A8BC(
                            (s32)(distance * invLength * 16384.0f));
                        width = influence->widthScale * along;
                        t = overlay99ProjectVector(normalZ, deltaX, sideD,
                                                      x, z);
                        if (t < 0.0f) {
                            t = -t;
                        }
                        if (t <= width) {
                            t = width - t;
                            edge = influence->edgeWidth;
                            if (t < edge) {
                                along = func_8002A8BC(
                                    (s32)((t * 16384.0f) / edge));
                                amount = func_8002A8C0(
                                             (s32)(t * 65536.0f *
                                                   invAngle) +
                                             phase) *
                                         (influence->intensity * along *
                                          wave);
                                point->z += (s32)amount;
                            }
                        }
                    }
                }
                point++;
            }
        }
    }
}

/*
 * The negated magnitude is spelled as an expression rather than written back
 * through `value`: the write-back form reuses the value's own register, where
 * the target computes the negation into a separate caller-saved temporary and
 * leaves the clamped value dead.
 */
void overlay99BuildHeightGrid(f32 scale, void *unused, s32 widthMinusOne,
                              s32 heightMinusOne, s32 extra4, s32 extra5) {
    Overlay99Vertex *point;
    s32 *widthPtr;
    s32 *heightPtr;
    s32 i;
    s32 value;

    point = D_8007BE88[D_8007BE84];
    if (point == 0) {
        return;
    }

    /* IDO homes an unreferenced named parameter to its incoming argument slot,
     * which is one instruction the target does not have; this empty test is
     * the only thing that stops the store. It is inert -- the second parameter
     * is dead in the target too -- and is recorded in docs/cleanup-queue.md.
     * Its position is load-bearing: before the null test or at the segment
     * loop the store comes back, and after the height grid is sized it also
     * exchanges the point/value pool colours. */
    if (unused != 0) {
    }
    widthPtr = &gOverlay99GridWidth;
    heightPtr = &gOverlay99GridHeight;
    *widthPtr = widthMinusOne + 1;
    *heightPtr = heightMinusOne + 1;
    gOverlay99WidthMinusOne = widthMinusOne;
    gOverlay99HeightMinusOne = heightMinusOne;
    gOverlay99Arg4 = extra4;
    gOverlay99Arg5 = extra5;

    i = *heightPtr * *widthPtr;
    while (i--) {
        point->z = 5;
        point++;
    }

    i = gOverlay99SegmentCount;
    while (i--) {
        overlay99ApplySegment((Overlay99Influence *)&gOverlay99Segments[i], scale);
    }

    widthPtr = &gOverlay99GridWidth;
    heightPtr = &gOverlay99GridHeight;
    i = *heightPtr * *widthPtr;
    point = D_8007BE88[D_8007BE84];
    while (i--) {
        value = point->z - 5;
        if (value < 0) {
            if (value < -40) {
                value = -40;
            }
            point->r = 0;
            point->g = 0;
            point->b = 0;
            point->a = -value;
        } else if (value > 0) {
            if (value > 40) {
                value = 40;
            }
            point->r = 0xFF;
            point->g = 0xFF;
            point->b = 0xFF;
            point->a = value;
        } else {
            point->a = 0;
        }
        point++;
    }
}

/*
 * The declaration list is a frame census, not a style choice.  uopt homes
 * every declared local at a descending word from the top of the 0x148 frame,
 * in declaration order, whether or not the local is ever touched through the
 * stack.  Reading the target's own homes back gives eighteen scalar slots
 * around the five aggregates: three above `savedX`, five between `savedZ` and
 * `distances`, six between `sprite` and `sorted`, four below it.  The body
 * uses nine of them; the rest are declared as `unused*` because the census is
 * what is known -- which scalar sat in which slot is not, and is provably
 * free: permuting the nine live names across the eighteen slots leaves the
 * object byte-identical.  Only the counts and the aggregate boundaries matter.
 */
void overlay99RenderSortedEntries(Gfx **displayList, Mtx **matrices,
                                  void *vertices, Overlay99RenderState *state,
                                  f32 intensityScale) {
    s32 count;
    s32 i;
    s32 j;
    f32 savedX;
    f32 savedY;
    f32 savedZ;
    f32 invScale;
    Overlay99RenderEntry *entry;
    Overlay99TableOwner *owner;
    Overlay99Transform *transform;
    Overlay99Vec3 *vec;
    f32 distances[4];
    MtxF matrix;
    Overlay99CameraSprite sprite;
    Gfx *command;
    s32 unused0;
    s32 unused1;
    s32 unused2;
    s32 unused3;
    s32 unused4;
    Overlay99RenderEntry *sorted[4];
    s32 unused5;
    s32 unused6;
    s32 unused7;
    s32 unused8;

    entry = state->entries;
    owner = *state->tableOwner;
    count = 0;
    if (entry != 0) {
        i = 0;
        while ((i < state->entryCount) && (i != 4)) {
            vec = &owner->vectors[entry->tableIndex];
            distances[count] = camGetProjZ(vec->x, vec->y, vec->z);
            sorted[count] = entry;
            entry++;
            i++;
            count++;
        }
    }

    if (count <= 0) {
        return;
    }

    for (i = count - 1; i > 0; i--) {
        for (j = 0; j < i; j++) {
            if (distances[j + 1] < distances[j]) {
                invScale = distances[j];
                distances[j] = distances[j + 1];
                distances[j + 1] = invScale;
                entry = sorted[j];
                sorted[j] = sorted[j + 1];
                sorted[j + 1] = entry;
            }
        }
    }

    transform = camGetPtr();
    savedX = transform->x;
    savedY = transform->y;
    savedZ = transform->z;
    transform->x = 0.0f;
    transform->y = 0.0f;
    transform->z = gOverlay99TransformZ;
    func_80022E80(state);
    transform->x = savedX;
    transform->y = savedY;
    transform->z = savedZ;

    invScale = 1.0f / *state->unitScale;
    func_8002AA50(state, matrix);
    sprite.frame = (s16)(s32)(intensityScale * gOverlay99IntensityScale);
    sprite.angle = 0;
    sprite.divisor = 3;
    sprite.frameCount = 0x3333;
    sprite.matrixScale = 1.0f;

    command = *displayList;
    *displayList = command + 1;
    command->words.w1 = 0;
    command->words.w0 = 0xE7000000;
    command = *displayList;
    *displayList = command + 1;
    command->words.w0 = 0xFB000000; command->words.w1 = 0xFFFFFF00;

    for (i = 0; i < count; i++) {
        entry = sorted[i];
        vec = &owner->vectors[entry->tableIndex];
        sprite.transformScale = entry->scale * invScale;
        sprite.spriteData = entry->spriteData;
        mtxf_transform_point(matrix, vec->x, vec->y, vec->z,
                                         &sprite.x, &sprite.y, &sprite.z);
        func_80022FD4(displayList, matrices, vertices, state,
                                   state->opacity, &sprite, 14,
                                   state->mode39);
    }
}

/* Overlay 99 +0xBA4..+0xDDC, 142 words with no target padding. The shipped
 * relocation tables authenticate viGetCurrentSize, rcpClearZBuffer,
 * texDPInit, Arctanf, sqrtf, func_80009E78, and the local sorted-entry
 * renderer. */
void overlay99RenderSegments(Gfx **displayList, Mtx **matrices, void *vertices,
                             f32 scale) {
    f32 dx;
    f32 dz;
    s32 width;
    s32 height;
    f32 length;
    Overlay99Segment *segment;
    Overlay99RenderState *object;
    Gfx *command;
    s32 initialized;
    s32 i;

    i = 0;
    segment = gOverlay99Segments;
    initialized = 0;
    if (gOverlay99SegmentCount > 0) {
        do {
            object = segment->object;
            if (object != 0) {
                if (initialized == 0) {
                    viGetCurrentSize(&width, &height);
                    initialized = 1;
                    command = (*displayList)++;
                    command->words.w0 = 0xBC000806;
                    command->words.w1 = (u32)D_800D2FAC + 0x80000000;
                    rcpClearZBuffer(displayList, width, height, 0,
                                               0, width, height);
                    texDPInit(displayList);
                }

                dx = segment->x1 - segment->x0;
                dz = segment->z1 - segment->z0;
                object->rotation0 = 0;
                object->rotation1 = 0;
                object->heading = Arctanf(-dx, dz);
                dx *= scale;
                dz *= scale;
                object->x = segment->x0 + dx;
                object->y = segment->z0 + dz;
                object->z = 5.0f;
                length = sqrtf((dx * dx) + (dz * dz));
                if (segment->headingOffset != 0) {
                    object->heading += (s32)((f32)segment->headingOffset *
                                             65536.0f * scale);
                }
                object->flags = (s16)object->flags & ~0x400;
                func_80009E78(displayList, matrices, vertices,
                                           object);
                object->flags = (s16)object->flags | 0x400;
                overlay99RenderSortedEntries(displayList, matrices, vertices,
                                             object, length);
            }
            i++;
            segment++;
        } while (i < gOverlay99SegmentCount);
    }
}

#define O99_SHIFTL(v, s, w) ((u32)(((u32)(v) & ((0x01 << (w)) - 1)) << (s)))
#define O99_K0_TO_PHYSICAL(x) (u32)(((char *)(x) - 0x80000000))
#define O99_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define O99_MAX(a, b) (((a) > (b)) ? (a) : (b))

#define O99_CMD(pkt, a, b) {                                                \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = (u32)(a);                                                \
    _g->words.w1 = (u32)(b);                                                \
}
#define O99_DMA1P(pkt, c, s, l, p) {                                        \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = (O99_SHIFTL((c), 24, 8) | O99_SHIFTL((p), 16, 8) |        \
                    O99_SHIFTL((l), 0, 16));                                \
    _g->words.w1 = (u32)(s);                                                \
}
#define gDPPipeSyncO99(pkt) O99_CMD(pkt, 0xE7000000, 0)
#define gDPLoadSyncO99(pkt) O99_CMD(pkt, 0xE6000000, 0)
#define gDPSetPrimColorO99(pkt, m, l, r, g, b, a) {                         \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = (O99_SHIFTL(0xFA, 24, 8) | O99_SHIFTL((m), 8, 8) |       \
                    O99_SHIFTL((l), 0, 8));                                 \
    _g->words.w1 = (O99_SHIFTL(r, 24, 8) | O99_SHIFTL(g, 16, 8) |           \
                    O99_SHIFTL(b, 8, 8) | O99_SHIFTL(a, 0, 8));             \
}
#define gDPSetTileO99(pkt, line, tmem, tile, cmt, cms) {                    \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = O99_SHIFTL(0xF5, 24, 8) | O99_SHIFTL(0, 21, 3) |         \
                   O99_SHIFTL(2, 19, 2) | O99_SHIFTL((line), 9, 9) |        \
                   O99_SHIFTL((tmem), 0, 9);                                \
    _g->words.w1 = O99_SHIFTL((tile), 24, 3) | O99_SHIFTL((cmt), 18, 2) |   \
                   O99_SHIFTL((cms), 8, 2);                                 \
}
#define gDPLoadTileO99(pkt, tile, uls, ult, lrs, lrt) {                     \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = (O99_SHIFTL(0xF4, 24, 8) | O99_SHIFTL((uls), 12, 12) |   \
                    O99_SHIFTL((ult), 0, 12));                              \
    _g->words.w1 = (O99_SHIFTL((tile), 24, 3) | O99_SHIFTL((lrs), 12, 12) | \
                    O99_SHIFTL((lrt), 0, 12));                              \
}
#define gDPSetTileSizeO99(pkt, t, uls, ult, lrs, lrt) {                     \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = (O99_SHIFTL(0xF2, 24, 8) | O99_SHIFTL((uls), 12, 12) |   \
                    O99_SHIFTL((ult), 0, 12));                              \
    _g->words.w1 = (O99_SHIFTL((t), 24, 3) | O99_SHIFTL((lrs), 12, 12) |    \
                    O99_SHIFTL((lrt), 0, 12));                              \
}
#define gSPVertexO99(pkt, v, n, v0)                                         \
    O99_DMA1P(pkt, 0x04, v, ((((n) << 3) + ((n) << 1)) + 8) | ((v0) << 9),  \
              ((n) << 3) | ((u32)(v) & 6))
#define gSPPolygonO99(pkt, ptr, numTris, texEnabled) {                      \
    Gfx *_g = (Gfx *)(pkt);                                                 \
    _g->words.w0 = O99_SHIFTL((((numTris) - 1) << 4) | (texEnabled), 16, 8) | \
                   O99_SHIFTL(5, 24, 8) | O99_SHIFTL(((numTris) * 16), 0, 16); \
    _g->words.w1 = (u32)(ptr);                                              \
}

void func_overlay_099_F0000DDC_18DA38C(Gfx **displayList, Mtx **matrices,
                                       void *vertices, f32 scale, s32 columns,
                                       s32 rows, s32 triangles, s32 stepX,
                                       s32 stepY) {
    s32 i;
    s32 j;
    s32 x;
    s32 y;
    s32 uls;
    s32 ult;
    s32 lrs;
    s32 lrt;
    s32 width;
    s32 alpha;
    Overlay99Vertex *vtx;
    s32 unused;

    if (D_8007BE80 == NULL) {
        return;
    }
    width = columns + 1;
    alpha = (s32)((1.0f - scale) * 768.0f);
    if (alpha >= 0x100) {
        alpha = 0xFF;
    }
    camStandardPersp(displayList, matrices);
    gDPPipeSyncO99((*displayList)++);
    O99_CMD((*displayList)++, 0xED000000, 0x005003C0);
    O99_CMD((*displayList)++, 0xEF30000F, 0);
    O99_CMD((*displayList)++, 0xF7000000, 0x00010001);
    O99_CMD((*displayList)++, 0xF64FC3BC, 0);
    gDPPipeSyncO99((*displayList)++);
    O99_CMD((*displayList)++, 0xFB000000, 0xFFFFFF00);
    gDPSetPrimColorO99((*displayList)++, 0, 0, alpha, alpha, alpha, 255);
    O99_CMD((*displayList)++, 0xB6000000, 0x00010001);
    O99_CMD((*displayList)++, 0xFC45FE03, 0x1FFCFDFE);
    O99_CMD((*displayList)++, 0xEF182C0F, 0x0F0A4000);
    O99_CMD((*displayList)++, 0xFD10013F, D_800D2FAC);
    for (i = 0, y = 0; i < rows; i++, y += stepY) {
        for (j = 0, x = 0; j < columns; j += 2, x += stepX) {
            uls = O99_MAX(x - 1, 0);
            ult = O99_MAX(y - 1, 0);
            lrs = O99_MIN(x + stepX, 319);
            lrt = O99_MIN(y + stepY, 239);
            gDPSetTileO99((*displayList)++, ((((lrs - uls) + 1) * 2) + 7) >> 3,
                          0, 7, 2, 2);
            gDPLoadSyncO99((*displayList)++);
            gDPLoadTileO99((*displayList)++, 7, uls << 2, ult << 2, lrs << 2,
                           lrt << 2);
            gDPPipeSyncO99((*displayList)++);
            gDPSetTileO99((*displayList)++, ((((lrs - uls) + 1) * 2) + 7) >> 3,
                          0, 0, 2, 2);
            gDPSetTileSizeO99((*displayList)++, 0, 0, 0,
                              ((lrs - uls) - 1) << 2, ((lrt - ult) - 1) << 2);
            vtx = &D_8007BE88[D_8007BE84][i * width + j];
            gSPVertexO99((*displayList)++, O99_K0_TO_PHYSICAL(vtx), 3, 0);
            vtx = &D_8007BE88[D_8007BE84][(i + 1) * width + j];
            gSPVertexO99((*displayList)++, O99_K0_TO_PHYSICAL(vtx), 3, 3);
            gSPPolygonO99((*displayList)++, O99_K0_TO_PHYSICAL(gO99TrianglesReloc),
                          triangles, 1);
        }
    }
    texDPInit(displayList);
    O99_CMD((*displayList)++, 0xFA000000, 0xFFFFFFFF);
    overlay99RenderSegments(displayList, matrices, vertices, scale);
}
