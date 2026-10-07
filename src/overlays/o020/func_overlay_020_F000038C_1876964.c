#include "PR/ultratypes.h"

typedef struct Overlay20InitVertex {
    s16 x;
    s16 value;
    s16 y;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} Overlay20InitVertex;

typedef struct Overlay20TexCoords {
    s16 u;
    s16 v;
} Overlay20TexCoords;

typedef struct Overlay20Triangle {
    u8 flags;
    u8 vi0;
    u8 vi1;
    u8 vi2;
    Overlay20TexCoords uv0;
    Overlay20TexCoords uv1;
    Overlay20TexCoords uv2;
} Overlay20Triangle;

typedef struct Overlay20InitTexture {
    u8 pad00[6];
    u16 width;
    u16 height;
} Overlay20InitTexture;

typedef struct Overlay20InitGrid {
    s16 minX;
    s16 baseValue;
    s16 minY;
    s16 width;
    s16 height;
    s16 columns;
    s16 rows;
    s16 textureScaleX;
    s16 textureScaleY;
    u8 bufferIndex;
    u8 baseColor;
    f32 negativeScale;
    f32 positiveScale;
    f32 displacementScale;
    Overlay20InitVertex *buffers[2];
    Overlay20Triangle *triangles;
    Overlay20InitTexture *texture;
} Overlay20InitGrid;

extern void *func_overlay_020_F0000000_18765D8();

/*
 * Rewritten from the listing (lane w2-ovld, 2026-10-02; lane a-ovl3,
 * 2026-10-07): the buffers are tested and filled through grid->buffers[i]
 * and grid->triangles directly, the four u/v values are computed before the
 * vertex indices, and each triangle is written whole in field order. The
 * second triangle's indices come from `index++`, a copy of index + 9 and a
 * named index + 10; that gives the target's nine saved registers and its
 * size. Open: register naming (row/index and rows/size swap their
 * registers), the first field loads through the argument register, and the
 * index + 9 copy.
 * 2026-10-07 (lane c-near), 104 -> 103: the second triangle walks the vertex
 * index (`index += 9`, then `index` and a named `far` computed at the top),
 * so the surviving copy is of index + 9, as in the target, not of index + 1.
 * Open: index + 1 spans two uopt blocks here and is denied v1 (col's
 * register); the target computes it inside one block.
 */
#ifdef NON_MATCHING
Overlay20InitGrid *func_overlay_020_F000038C_1876964(Overlay20InitGrid *grid) {
    Overlay20InitVertex *vertex;
    Overlay20Triangle *tri;
    Overlay20InitTexture *texture;
    s32 size;
    s32 i;
    s32 row;
    s32 col;
    s32 index;
    s32 far;
    s32 u0;
    s32 u1;
    s32 v0;
    s32 v1;

    size = (grid->columns + 1) * (grid->rows + 1) * sizeof(Overlay20InitVertex);
    texture = grid->texture;
    for (i = 0; i < 2; i++) {
        if (grid->buffers[i] == NULL) {
            grid->buffers[i] = func_overlay_020_F0000000_18765D8(size, 0x87);
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

    size = (grid->columns * grid->rows) * (2 * sizeof(Overlay20Triangle));
    if (grid->triangles == NULL) {
        grid->triangles = func_overlay_020_F0000000_18765D8(size, 0x87);
    }
    tri = grid->triangles;
    if (tri != NULL) {
        for (row = 0; row < grid->rows; row++) {
            for (col = 0; col < grid->columns; col++) {
                u0 = ((grid->textureScaleX * col * texture->width) << 5) / grid->columns;
                u1 = ((grid->textureScaleX * (col + 1) * texture->width) << 5) / grid->columns;
                v0 = ((grid->textureScaleY * row * texture->height) << 5) / grid->rows;
                v1 = ((grid->textureScaleY * (row + 1) * texture->height) << 5) / grid->rows;
                index = col & 7;
                far = index + 10;
                tri[0].flags = 0x40;
                tri[0].vi0 = index;
                tri[0].vi1 = index + 1;
                tri[0].vi2 = index + 9;
                tri[0].uv0.u = u0;
                tri[0].uv0.v = v0;
                tri[0].uv1.u = u1;
                tri[0].uv1.v = v0;
                tri[0].uv2.u = u0;
                tri[0].uv2.v = v1;
                tri[1].flags = 0x40;
                tri[1].vi0 = index + 1;
                index += 9;
                tri[1].vi1 = index;
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
        func_overlay_020_F0000000_18765D8(grid);
        grid = NULL;
    }
    return grid;
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o020/func_overlay_020_F000038C_1876964/func_overlay_020_F000038C_1876964.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_020_F000038C_1876964:start
 * symbol: func_overlay_020_F000038C_1876964
 * score: 103 differing words
 * frame: 0x40
 * relocations: 3
 * first-mismatch: +0x4
 * summary: Walking vertex index (index += 9, named far): copy now of index+9 as shipped, 104 to 103. Left: index+1 spans two uopt blocks and is denied v1.
 * PLATEAU-HANDOFF:func_overlay_020_F000038C_1876964:end
 */
