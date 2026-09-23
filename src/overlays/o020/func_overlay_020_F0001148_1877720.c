#include "PR/ultratypes.h"

typedef struct Overlay20TailVector {
    f32 x;
    f32 y;
    f32 z;
} Overlay20TailVector;

typedef struct Overlay20TailVertex {
    s16 x;
    s16 value;
    s16 y;
    u8 red;
    u8 green;
    u8 blue;
    u8 pad9;
} Overlay20TailVertex;

typedef struct Overlay20TailGrid {
    s16 minX;
    s16 baseValue;
    s16 minY;
    s16 width;
    s16 height;
    s16 columns;
    s16 rows;
    u8 pad0E[4];
    u8 bufferIndex;
    u8 baseColor;
    f32 negativeScale;
    f32 positiveScale;
    f32 displacementScale;
    Overlay20TailVertex *buffers[2];
} Overlay20TailGrid;

extern f32 sqrtf(f32 value);
extern f32 gOverlay20TailXLimit;
extern f32 gOverlay20TailYLimit;

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
