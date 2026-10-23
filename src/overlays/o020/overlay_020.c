#include "overlays/overlay_020.h"

/*
 * Overlay 20: one translation unit in ROM order (ADR 0006). All fifteen
 * functions are ORT exports.
 */

/* DKR v77/v80 and JFG contain no exact donor. Not recursive: all four calls
 * are shipped SYMBOL records to the resident mmFree, so this frees the three
 * child buffers and then the node itself. */
void overlay20ReleaseTree(Overlay20TreeNode *node) {
    if (node != NULL) {
        if (node->child0 != NULL) {
            mmFree(node->child0);
            node->child0 = NULL;
        }
        if (node->child1 != NULL) {
            mmFree(node->child1);
            node->child1 = NULL;
        }
        if (node->child2 != NULL) {
            mmFree(node->child2);
            node->child2 = NULL;
        }
        mmFree(node);
    }
}

/* DKR v77/v80 has the generic optional-release idiom, but no exact donor. */
void overlay20ReleaseHandle(Overlay20State *state) {
    if (state->handle != 0) {
        func_overlay_020_F0000000_18765D8Reloc((void *)state->handle);
    }
}


Overlay20Resource *overlay20ConfigureResource(
    Overlay20Resource *resource, s32 x, s32 y, s32 width, s32 height,
    s32 stepX, s32 columns, s32 rows, void *owner, s32 value0E,
    s32 value10, s32 start, s32 current, s32 end, s32 scaleDivisor) {
    if (resource == 0) {
        resource = mmAlloc(0x30, 0x87);
        if (resource != 0) {
            resource->buffers[0] = 0;
            resource->buffers[1] = 0;
            resource->buffers[2] = 0;
        }
    } else if ((columns != resource->columns) || (rows != resource->rows)) {
        mmFree(resource->buffers[0]);
        mmFree(resource->buffers[1]);
        mmFree(resource->buffers[2]);
        resource->buffers[0] = 0;
        resource->buffers[1] = 0;
        resource->buffers[2] = 0;
    }

    if (resource != 0) {
        resource->x = x;
        resource->y = y;
        resource->width = width;
        resource->height = height;
        resource->stepX = stepX;
        resource->columns = columns;
        resource->rows = rows;
        resource->value0E = value0E;
        resource->value10 = value10;
        resource->current = current;
        resource->start = (f32)(current - start);
        resource->end = (f32)(end - current);
        resource->scale = 1.0f;
        if (scaleDivisor > 0) {
            resource->scale /= (f32)scaleDivisor;
        }
        resource->owner = owner;
        resource = func_overlay_020_F000038C_1876964Reloc(resource);
    }
    return resource;
}

/* Two coupled allocator facts fix the temp ring. The bound check reads
 * context->count directly, so the value is a ugen temp rather than a uopt-
 * coloured web, which pops t6 before the owner load; and the entry table is an
 * array of eight-byte pairs, so the index scales in one step instead of the
 * multiply-then-scale pair that popped a second, invisible temp. */
void overlay20UpdateObjectResource(Overlay20Object *object,
                                   Overlay20Config *config) {
    s32 baseX;
    s32 baseY;
    s32 objectY;
    s32 width;
    s32 height;
    void *owner;
    Overlay20LookupResult *lookup;
    Overlay20Context *context;

    context = trackGetTrack();
    if ((config->useLookup != 0) &&
        ((lookup = func_8000FEEC(object->lookupIndex)), lookup != 0)) {
        baseX = lookup->x0;
        objectY = (s32)object->y;
        baseY = lookup->y0;
        width = lookup->x1 - lookup->x0;
        height = lookup->y1 - lookup->y0;
    } else {
        baseX = (s32)object->x;
        objectY = (s32)object->y;
        baseY = (s32)object->z;
        width = config->width;
        height = config->height;
    }

    if ((config->entryIndex >= 0) &&
        (config->entryIndex < context->count)) {
        owner = context->entries[config->entryIndex].owner;
    } else {
        owner = *object->fallbackEntry;
    }

    object->resource = func_overlay_020_F00000A8_1876680Reloc(
        object->resource, baseX, objectY, baseY, width, height,
        config->columns, config->rows, owner, config->value0E,
        config->value0F, config->start, config->current, config->end,
        config->scaleDivisor);
    object->radius = sqrtf((f32)((width * width) +
                                               (height * height)));
}


/*
 * Rewritten from the listing (lanes w2-ovld, a-ovl3, c-near, d-mid1, g-3,
 * i-2, k-2, m-4, x-1). The vertex index `i = col & 7`, the two index
 * locals `next` (index + 9, the second triangle's middle vertex, kept as the
 * shipped copy) and `far` (index + 10) are computed between v0 and v1, so
 * ugen emits them ahead of v1's division as shipped. Spelling far from
 * `col & 7` costs one more outside-variable load, which moves uopt's -varref
 * block cut to just before tri[0].vi2: index + 9 then spans two blocks and
 * takes s2 as shipped. `next` is its own local, not the allocation `size`,
 * so its save ties far's and wins on web number (s4/s5 as shipped).
 * Stand-in (flagged): the dead `i++` after tri[1].vi0 is a redefinition of
 * i in the use block; without it copy propagation forwards next and far into
 * their stores and the function is 16 bytes short. A natural statement that
 * redefines i there has not been found (shard, 2026-10-09 lane x-1).
 */
Overlay20InitGrid *func_overlay_020_F000038C_1876964(Overlay20InitGrid *grid) {
    Overlay20InitVertex *vertex;
    Overlay20Triangle *tri;
    Overlay20InitTexture *texture;
    s32 size;
    s32 i;
    s32 row;
    s32 col;
    s32 far;
    s32 next;
    s32 u0;
    s32 u1;
    s32 v0;
    s32 v1;

    size = (grid->columns + 1) * (grid->rows + 1) * sizeof(Overlay20InitVertex);
    texture = grid->texture;
    for (i = 0; i < 2; i++) {
        if (grid->buffers[i] == NULL) {
            grid->buffers[i] = mmAlloc(size, 0x87);
        }
        vertex = grid->buffers[i];
        if (vertex != NULL) {
            for (row = 0; row <= grid->rows; row++) {
                for (col = 0; col <= grid->columns; col++) {
                    vertex->x = grid->minX + (col * grid->width) / grid->columns;
                    vertex->value = grid->baseValue;
                    vertex->y = grid->minY + (row * grid->height) / grid->rows;
                    vertex->red = 0xFF;
                    vertex->green = 0xFF;
                    vertex->blue = 0xFF;
                    vertex->alpha = 0xFF;
                    vertex++;
                }
            }
        }
    }

    size = grid->columns * grid->rows * 2 * sizeof(Overlay20Triangle);
    if (grid->triangles == NULL) {
        grid->triangles = mmAlloc(size, 0x87);
    }
    tri = grid->triangles;
    if (tri != NULL) {
        for (row = 0; row < grid->rows; row++) {
            for (col = 0; col < grid->columns; col++) {
                u0 = ((grid->textureScaleX * col * texture->width) << 5) / grid->columns;
                u1 = (((col + 1) * grid->textureScaleX * texture->width) << 5) / grid->columns;
                v0 = ((grid->textureScaleY * row * texture->height) << 5) / grid->rows;
                i = col & 7;
                next = i + 9;
                far = (col & 7) + 10;
                v1 = (((row + 1) * grid->textureScaleY * texture->height) << 5) / grid->rows;
                tri[0].flags = 0x40;
                tri[0].vi0 = i;
                tri[0].vi1 = i + 1;
                tri[0].vi2 = i + 9;
                tri[0].uv0.u = u0;
                tri[0].uv0.v = v0;
                tri[0].uv1.u = u1;
                tri[0].uv1.v = v0;
                tri[0].uv2.u = u0;
                tri[0].uv2.v = v1;
                tri[1].flags = 0x40;
                tri[1].vi0 = i + 1;
                i++;
                tri[1].vi1 = next;
                tri[1].vi2 = far;
                tri[1].uv0.u = u1;
                tri[1].uv0.v = v0;
                tri[1].uv1.u = u0;
                tri[1].uv1.v = v1;
                tri[1].uv2.u = u1;
                tri[1].uv2.v = v1;
                tri += 2;
            }
        }
    }

    grid->bufferIndex = 1;
    if (grid->buffers[0] == NULL || grid->buffers[1] == NULL || grid->triangles == NULL) {
        func_overlay_020_F0000000_18765D8Reloc(grid);
        grid = NULL;
    }
    return grid;
}

/* Overlay 20 table-1 record 13 resolves the call at function +0x38 through
 * ORT 206 to the resident texDPTextureX. */

#define OVERLAY20_SHIFTL(value, shift, width)                              \
    ((u32)(((u32)(value) & ((1U << (width)) - 1)) << (shift)))

/* PROVENANCE: source-shape crosswalk from Diddy Kong Racing
 * include/PR/gbi.h:gDma1p and include/f3ddkr.h:gSPPolygon. Mickey's ten-byte
 * vertex records decide the length expression here; the published DKR macros
 * only supply the command construction idiom. */
#define OVERLAY20_VERTEX(packet, address, count, lengthFlags)               \
    {                                                                       \
        Overlay20Command *_g = (Overlay20Command *)(packet);                \
        _g->w0 = OVERLAY20_SHIFTL(4, 24, 8) |                               \
                 OVERLAY20_SHIFTL(((count) << 3) |                          \
                                       ((u32)(address) & 6),                \
                                   16, 8) |                                 \
                 OVERLAY20_SHIFTL((((count) << 3) + ((count) << 1) + 8) |   \
                                       (lengthFlags),                       \
                                   0, 16);                                  \
        _g->w1 = (u32)(address);                                            \
    }

#define OVERLAY20_POLYGON(packet, address, count, textured)                 \
    {                                                                       \
        Overlay20Command *_g = (Overlay20Command *)(packet);                \
        _g->w0 = OVERLAY20_SHIFTL((((count) - 1) << 4) | (textured),        \
                                   16, 8) |                                 \
                 OVERLAY20_SHIFTL(5, 24, 8) |                               \
                 OVERLAY20_SHIFTL((count) << 4, 0, 16);                    \
        _g->w1 = (u32)(address);                                            \
    }

/* Mickey's 0x90-byte frame proves a seven-entry chunk array. Keeping it after
 * the six preceding scalar/cursor declarations gives the retail sp+0x5C home;
 * all 134 instruction words and the helper relocation are exact. */
void overlay20BuildTileCommands(Overlay20Command **commands,
                                Overlay20TileSource *source, s32 arg2) {
    s32 width;
    s32 remainingRows;
    s32 chunkCount;
    s32 *writeCursor;
    s32 *readCursor;
    s32 textureOffset;
    s32 chunks[7];
    s32 outputOffset;
    s32 chunkWidth;
    s32 doubledWidth;

    texDPTextureX(commands, source->resource, arg2, 0);
    width = source->width;
    chunkCount = 0;
    writeCursor = chunks;
    if (width != 0) {
        do {
            if (width >= 9) {
                *writeCursor = 8;
                chunkCount++;
                writeCursor++;
                width -= 8;
            } else {
                *writeCursor = width;
                chunkCount++;
                writeCursor++;
                width = 0;
            }
        } while (width != 0);
    }

    textureOffset = source->textureOffsets[source->variant];
    remainingRows = source->rows;
    outputOffset = source->outputOffset;
    while (remainingRows--) {
        width = 0;
        if (chunkCount > 0) {
            readCursor = chunks;
            do {
                chunkWidth = *readCursor;
                OVERLAY20_VERTEX((*commands)++,
                                 textureOffset + (s32)0x80000000,
                                 chunkWidth + 1, 0);
                width++;
                readCursor++;
                OVERLAY20_VERTEX(
                    (*commands)++,
                    textureOffset + source->width * 10 + (s32)0x8000000A,
                    chunkWidth + 1, 0x1200);
                doubledWidth = chunkWidth << 1;
                OVERLAY20_POLYGON((*commands)++,
                                  outputOffset + (s32)0x80000000,
                                  doubledWidth, 1);
                textureOffset += chunkWidth * 10;
                outputOffset += doubledWidth << 4;
            } while (width != chunkCount);
        }
        textureOffset += 10;
    }
}

#define OVERLAY20_PRIM_SHIFTL(value, shift, width) \
    (((u32)(value) & ((1U << (width)) - 1U)) << (shift))

#define OVERLAY20_SET_PRIM_COLOR(packet, alpha)                       \
    {                                                                  \
        Overlay20Gfx *macroCommand = (Overlay20Gfx *)(packet);          \
        macroCommand->w0 = 0xFA000000;                                  \
        macroCommand->w1 = OVERLAY20_PRIM_SHIFTL(255, 24, 8) |               \
                           OVERLAY20_PRIM_SHIFTL(255, 16, 8) |               \
                           OVERLAY20_PRIM_SHIFTL(255, 8, 8) |                \
                           OVERLAY20_PRIM_SHIFTL((alpha), 0, 8);             \
    }

/* DKR v77/v80 and JFG have generic GBI relatives, but no exact donor. */
void overlay20DrawResource(Overlay20Gfx **gfxP, Overlay20Object *object) {
    void *resource;
    s32 mode;

    resource = object->resource;
    if (resource != NULL) {
        mode = 10;
        if (object->alpha < 255) {
            OVERLAY20_SET_PRIM_COLOR((*gfxP)++, object->alpha);
            mode = 14;
        }
        func_overlay_020_F00007C4_1876D9CReloc(gfxP, resource, mode);
        OVERLAY20_SET_PRIM_COLOR((*gfxP)++, 255);
    }
}


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
                        distance = sqrtf(distanceSquared);
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

/* DKR v77/v80 and JFG have no exact donor for this guarded release. */
void overlay20ReleaseEntry(Overlay20ReleaseOwner *owner) {
    if (owner->entry != 0 && owner->marked != 0) {
        func_overlay_020_F0000A68_1877040Reloc((void *)owner->entry);
    }
    owner->marked = 0;
}

/* DKR v77/v80 and JFG have no exact donor for this guarded state setter. */
void overlay20MarkNested(Overlay20NestedOwner *owner) {
    if (owner->nested != NULL) {
        owner->marked = 1;
    }
}

#define gOverlay20LifetimeScale D_0.lifetimeScale

/* DKR v77/v80 and JFG have no exact donor for this entry allocator. */
Overlay20Entry *overlay20ConfigureEntry(Overlay20Entry *entry, f32 x, f32 y,
                                        f32 radius, s32 id, f32 lifetime,
                                        f32 scale, f32 ratio) {
    s32 index;
    u32 bit;

    if (entry == NULL) {
        index = gOverlay20EntryCount;
        if (index >= 32 || gOverlay20ActiveBits == -1) {
            entry = NULL;
        } else {
            bit = 1;
            entry = gOverlay20Pool;
            if (1 & gOverlay20ActiveBits) {
                do {
                    entry++;
                    bit <<= 1;
                } while (bit & gOverlay20ActiveBits);
            }
            gOverlay20Entries[index] = entry;
            gOverlay20EntryCount = index + 1;
            gOverlay20ActiveBits |= bit;
        }
    } else {
        id = entry->phase;
    }

    if (entry != NULL) {
        entry->x = x;
        entry->y = y;
        entry->radius = radius;
        entry->radiusSquared = radius * radius;
        entry->minX = (s32)(x - radius);
        entry->minY = (s32)(y - radius);
        entry->maxX = (s32)(x + radius);
        entry->maxY = (s32)(y + radius);
        entry->phase = id;
        entry->lifetime = (s32)(lifetime * gOverlay20LifetimeScale);
        entry->frequency = 65536.0f / scale;
        entry->radiusRatio = ratio / radius;
    }
    return entry;
}

/* Exact at +0xF78; DKR v77/v80 and JFG have no exact donor for this wrapper. */
void overlay20CreateEntry(Overlay20Owner *owner, Overlay20EntryConfig *config) {
    owner->entry = func_overlay_020_F0000E28_1877400Reloc(
        owner->entry, owner->x, owner->z, (f32)config->heading, config->mode,
        (f32)config->scale * gOverlay20Scale, (f32)config->value0,
        (f32)config->value1);
}


/* PROVENANCE: indexed search and list-compaction loops adapted from Diddy Kong
 * Racing's published src/weather.c::lensflare_override_remove. Mickey's owner
 * offset, arrays, marker cleanup, relocations, and target bytes remain
 * authoritative. */
/* Matched 2026-10-01. The count is read from its global at every use and
 * decremented in place, and the search and the compaction index the one entry
 * array. The earlier candidate copied the count into the parameter, carried
 * the decremented bound in a local and named the compaction array separately;
 * that left a count web holding the first colour across the compaction loop,
 * so its limit could not take it. The search stays bottom-tested: the `for`
 * spelling schedules two words differently. The marker walk still reuses the
 * parameter as its cursor; a separate cursor local costs nine words. The
 * parameter is the owner object the resident caller passes, so it is a byte
 * pointer rather than an s32; the code is the same (2026-10-22). */
void overlay20RemoveEntry(u8 *owner) {
    void *entry;
    s32 i;

    entry = ((Overlay20RemoveOwner *)owner)->entry;
    if (entry == NULL) {
        return;
    }
    i = 0;
    if (gOverlay20EntryCount > 0) {
        do {
            if (entry == gOverlay20Entries[i]) {
                break;
            }
            i++;
        } while (i < gOverlay20EntryCount);
    }
    if (i >= gOverlay20EntryCount) {
        return;
    }
    gOverlay20EntryCount--;
    for (; i < gOverlay20EntryCount; i++) {
        gOverlay20Entries[i] = gOverlay20Entries[i + 1];
    }

    owner = &gOverlay20MarkerEnd;
    i = 31;
    do {
        if (owner != NULL) {
            gOverlay20ActiveBits &= ~(1U << i);
            return;
        }
        owner -= 0x24;
    } while (i--);
}


/* Exact at +0x10EC. DKR has only generic update-rate/velocity relatives. */
void overlay20AdvanceEntries(s32 amount) {
    s32 i;
    Overlay20Entry *current;

    i = 0;
    for (; i < gOverlay20EntryCount; i++) {
        current = gOverlay20Entries[i];
        current->phase += current->lifetime * amount;
    }
}

/* Built at -mips2 (mk/overlays.mk): the target keeps a slot between every FP
 * compare and its bc1 branch, which as1 inserts only below MIPS III.
 * x and y are clamped in place (one web each with their argument copies);
 * width and height are named so their conversions are hoisted ahead of the
 * divisors; stride is defined beside the vertex index, which gives its web
 * the save that outranks upperTriangle; cornerValue carries vertex[1] in the
 * upper arm as it does in the lower; vertex[stride] is subscripted with no
 * row pointer, and planeX is assigned after cornerValue. Every f32, pointer
 * and s32 local takes a stack slot in declaration order (L99), so the order
 * below, including the unused nextRow, places the six spilled homes. */
f32 func_overlay_020_F0001148_1877720(Overlay20TailGrid *grid, f32 x, f32 y,
    Overlay20TailVector *normal) {
    Overlay20TailVertex *vertex;
    Overlay20TailVertex *nextRow;
    s32 column;
    s32 row;
    s32 stride;
    s32 upperTriangle;
    f32 width;
    f32 height;
    f32 planeX;
    f32 baseValue;
    volatile f32 rowValue;
    f32 cornerValue;
    f32 normalX;
    f32 normalY;
    f32 normalZ;
    f32 cellWidth;
    f32 cellHeight;
    f32 result;
    f32 length;

    width = (f32)grid->width;
    height = (f32)grid->height;
    cellWidth = width / (f32)grid->columns;
    cellHeight = height / (f32)grid->rows;
    result = (f32)grid->baseValue;
    x -= (f32)grid->minX;
    y -= (f32)grid->minY;
    if (x < 0.0f) {
        x = 0.0f;
    } else if (width <= x) {
        x = width - gOverlay20TailXLimit;
    }
    if (y < 0.0f) {
        y = 0.0f;
    } else if (height <= y) {
        y = height - gOverlay20TailYLimit;
    }

    column = (s32)(x / cellWidth);
    row = (s32)(y / cellHeight);
    x -= (f32)column * cellWidth;
    y -= (f32)row * cellHeight;
    upperTriangle = 0;
    if ((y != cellHeight) &&
        (x < (((cellHeight - y) / cellHeight) * cellWidth))) {
        upperTriangle = 1;
    }

    stride = grid->rows + 1;
    vertex = grid->buffers[grid->bufferIndex] + (column + (row * stride));
    if (upperTriangle != 0) {
        baseValue = (f32)vertex->value;
        rowValue = (f32)vertex[stride].value;
        cornerValue = (f32)vertex[1].value;
        planeX = 0.0f;
        normalX = (baseValue - cornerValue) * cellHeight;
        normalY = cellHeight * cellWidth;
        normalZ = (baseValue - rowValue) * cellWidth;
    } else {
        baseValue = (f32)vertex[1].value;
        rowValue = (f32)vertex[stride].value;
        cornerValue = (f32)vertex[stride + 1].value;
        planeX = cellWidth;
        normalX = (rowValue - cornerValue) * cellHeight;
        normalY = cellHeight * cellWidth;
        normalZ = (baseValue - cornerValue) * cellWidth;
    }

    length = sqrtf((normalX * normalX) + (normalY * normalY) +
                   (normalZ * normalZ));
    if ((length != 0.0f) && (normalY != 0.0f)) {
        normalX /= length;
        normalZ /= length;
        normalY /= length;
        result = -(((normalX * x) + (normalZ * y)) -
                   ((planeX * normalX) + (baseValue * normalY))) /
                 normalY;
    }
    if (normal != NULL) {
        normal->x = normalX;
        normal->y = normalY;
        normal->z = normalZ;
    }
    return result;
}
