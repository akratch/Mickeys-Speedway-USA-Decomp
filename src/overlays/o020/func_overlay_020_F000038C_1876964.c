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

    size = grid->columns * grid->rows * 2 * sizeof(Overlay20Triangle);
    if (grid->triangles == NULL) {
        grid->triangles = func_overlay_020_F0000000_18765D8(size, 0x87);
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
        func_overlay_020_F0000000_18765D8(grid);
        grid = NULL;
    }
    return grid;
}
